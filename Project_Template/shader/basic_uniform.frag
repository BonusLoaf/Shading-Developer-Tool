#version 460

in vec3 Position;
in vec3 Normal;

in vec3 Vec;



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

//layout(binding=5) uniform samplerCube SkyBoxTex;
layout(binding=1) uniform sampler2D PyTex;
layout(binding=2) uniform sampler2D StaffTex;
layout(binding=3) uniform sampler2D DirtTex;
layout(binding=4) uniform sampler2D SandTex;
layout( binding=0 ) uniform sampler2D Texture0;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

vec3 TexColor = texture(PyTex, TexCoord).rgb;
vec4 TexPyramid = texture(PyTex, TexCoord);
vec4 TexOverlay = texture(DirtTex, TexCoord);

uniform float Weight[5];
uniform float EdgeThreshold;
uniform int Pass;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);


vec3 blinnPhongModel(vec3 position, vec3 normal)
{



if(texID == 1)
{
TexColor = texture(PyTex, TexCoord).rgb;
}
else if(texID == 2)
{
TexColor = texture(StaffTex, TexCoord).rgb;
}
else if(texID == 3)
{
TexColor = texture(SandTex, TexCoord).rgb;
}



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



//Lumincance function
float luminance( vec3 color )
{
 return dot(lum,color);
}


vec4 pass1()
{
 return vec4(blinnPhongModel( Position, normalize(Normal) ),1.0);
}

//Calculates luminance for each pixel
vec4 pass2()
{
 ivec2 pix = ivec2( gl_FragCoord.xy );
 vec4 sum = texelFetch(Texture0, pix, 0) * Weight[0];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,1) ) * Weight[1];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-1) ) * Weight[1];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,2) ) * Weight[2];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-2) ) * Weight[2];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,3) ) * Weight[3];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-3) ) * Weight[3];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,4) ) * Weight[4];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(0,-4) ) * Weight[4];
 return sum;
}


vec4 pass3()
{
 ivec2 pix = ivec2( gl_FragCoord.xy );
 vec4 sum = texelFetch(Texture0, pix, 0) * Weight[0];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(1,0) ) * Weight[1];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(-1,0) ) * Weight[1];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(2,0) ) * Weight[2];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(-2,0) ) * Weight[2];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(3,0) ) * Weight[3];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(-3,0) ) * Weight[3];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(4,0) ) * Weight[4];
 sum += texelFetchOffset( Texture0, pix, 0, ivec2(-4,0) ) * Weight[4];
 return sum;
}




void main() {


if(texID > 0)
{


if( Pass == 1 )
{
FragColor = pass1();
}
 else if( Pass == 2 )
{
FragColor = pass2();
}
 else if( Pass == 3 )
{
FragColor = pass3();
}





}
else
{
//vec3 texColor = texture(SkyBoxTex, normalize(Vec)).rgb;
//
////Set skybox shader
//FragColor = vec4 (texColor,1.0f);
}

}
