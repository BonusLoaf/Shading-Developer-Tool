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




layout(binding=2) uniform sampler2D StaffTex;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

vec3 TexColor = texture(StaffTex, TexCoord).rgb;



//vec3 blinnPhongModelWithNormal(vec3 LightDir, vec3 normal)
//{
//
//
//
//
//TexColor = mix(TexPyramid.rgb, TexOverlay.rgb, TexOverlay.a);
//
//
////Ambient
//vec3 ambient = Light.La * TexColor;
//
////Diffuse
//vec3 s = normalize(vec3(Light.Position) - LightDir);
//
//float sDotN = dot(LightDir,normal);
//
//vec3 diffuse = TexColor * sDotN;
//
////Specular
//
////Calculate without shininess
//vec3 specular = Material.Ks * Light.L * sDotN;
//
//
////Shininess
//if(sDotN > 0.0)
//{
//
////Half Vector
//vec3 h = normalize(ViewDir+LightDir);
//
//
////Calculate specular with Shininess
//specular = Material.Ks * pow(max(dot(h,normal),0.0), Material.Shininess);
//
//}
//
//return ambient + Light.L * (diffuse + specular);
//
//
//
//}



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

//Calculate Normal Map
//vec3 norm = texture(NormalMapTex, TexCoord).xyz;
//Set it between a range of 0 and 1
//norm.xy = 2.0 * norm.xy - 1.0f;

//Add point light to spot light
vec3 fullLightModel = blinnPhongModel(Position, normalize(Normal));

//Set pyramid shader
FragColor = vec4(fullLightModel,1.0);



}
