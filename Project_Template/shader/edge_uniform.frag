#version 460

in vec3 Position;
in vec3 Normal;

in vec3 Vec;


in vec3 LightDir;
in vec3 ViewDir;

uniform struct lightInfo
{
vec4 Position;
vec3 La;
vec3 L;
}Light;

uniform int texID;

uniform struct materialInfo
{
vec3 Ka;
vec3 Kd;
vec3 Ks;
float Shininess;
}Material;


uniform struct SpotLightInfo {
vec3 Position;
vec3 L;
vec3 La;
vec3 Direction;
float Exponent;
float Cutoff;
} Spot;

layout(binding=0) uniform samplerCube SkyBoxTex;
layout(binding=1) uniform sampler2D PyTex;
layout(binding=2) uniform sampler2D StaffTex;
layout(binding=3) uniform sampler2D NormalMapTex;
layout(binding=4) uniform sampler2D DirtTex;
layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

vec3 TexColor = texture(PyTex, TexCoord).rgb;

vec4 TexPyramid = texture(PyTex, TexCoord);

vec4 TexOverlay = texture(DirtTex, TexCoord);

vec3 blinnPhongModelWithNormal(vec3 LightDir, vec3 normal)
{




TexColor = mix(TexPyramid.rgb, TexOverlay.rgb, TexOverlay.a);


//Ambient
vec3 ambient = Light.La * TexColor;

//Diffuse
vec3 s = normalize(vec3(Light.Position) - LightDir);

float sDotN = dot(LightDir,normal);

vec3 diffuse = TexColor * sDotN;

//Specular

//Calculate without shininess
vec3 specular = Material.Ks * Light.L * sDotN;


//Shininess
if(sDotN > 0.0)
{

//Half Vector
vec3 h = normalize(ViewDir+LightDir);


//Calculate specular with Shininess
specular = Material.Ks * pow(max(dot(h,normal),0.0), Material.Shininess);

}

return ambient + Light.L * (diffuse + specular);



}

vec3 blinnPhongSpot( vec3 position, vec3 n ) {



//Ambient
vec3 ambient = Spot.La * Material.Ka;



//Diffuse
vec3 s = normalize(vec3(Spot.Position) - position);

float cosAng = dot(-s, normalize(Spot.Direction));

float angle = acos(cosAng);

float spotScale = 0.0f;

vec3 diffuse = vec3(0.0);

vec3 specular = vec3(0.0);


if(angle < Spot.Cutoff)
{

spotScale = pow(cosAng, Spot.Exponent);
float sDotN = dot(s,n);

vec3 diffuse = Material.Kd * sDotN;

if(sDotN > 0.0)
{
vec3 v = normalize(-position.xyz);
vec3 h = normalize(v+s);
specular = Material.Ks * pow(max(dot(h,n),0.0),Material.Shininess);

}
return ambient + spotScale * Spot.L * (diffuse + specular);
}


return ambient + spotScale * Spot.L * (diffuse + specular);

}


vec3 blinnPhongModel(vec3 position, vec3 normal)
{


TexColor = texture(StaffTex, TexCoord).rgb;




//Ambient
vec3 ambient = Light.La * TexColor;

//Diffuse
vec3 s = normalize(vec3(Light.Position) - position);

float sDotN = dot(s,normal);

vec3 diffuse = TexColor * sDotN;

//Specular

//Calculate without shininess
vec3 specular = Material.Ks * Light.L * sDotN;

vec3 v = normalize(-position.xyz);




//Shininess
if(sDotN > 0.0)
{

//Half Vector
vec3 h = normalize(v+s);

//Calculate specular with Shininess
specular = Material.Ks * pow(max(dot(h,normal),0.0), Material.Shininess);

}

return ambient + Light.L * (diffuse + specular);



}


void main() {


if(texID > 0)
{

if(texID == 1)
{

//Calculate Normal Map
vec3 norm = texture(NormalMapTex, TexCoord).xyz;
//Set it between a range of 0 and 1
norm.xy = 2.0 * norm.xy - 1.0f;

//Add point light to spot light
vec3 fullLightModel = blinnPhongSpot(Position, normalize(Normal)) + blinnPhongModelWithNormal(LightDir, normalize(norm));

//Set pyramid shader
    FragColor = vec4(fullLightModel,1.0);
}
else
{
//Set staff shader
FragColor = vec4 (blinnPhongModel(Position, normalize(Normal)),1.0);
}


}
else
{
vec3 texColor = texture(SkyBoxTex, normalize(Vec)).rgb;

//Set skybox shader
FragColor = vec4 (texColor,1.0f);
}

}
