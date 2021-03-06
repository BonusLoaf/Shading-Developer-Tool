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


layout(binding=1) uniform sampler2D TreeTex;
layout(binding=2) uniform sampler2D SphinxTex;
layout(binding=3) uniform sampler2D PondTex;
layout(binding=4) uniform sampler2D SandTex;
layout( binding=0 ) uniform sampler2D BlurTex;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;

vec3 TexColor = texture(PondTex, TexCoord).rgb;



uniform float Weight[5];
uniform float EdgeThreshold;
uniform int Pass;

const vec3 lum = vec3(0.2126, 0.7152, 0.0722);


vec3 blinnPhongModel(vec3 position, vec3 normal)
{



if(texID == 1)
{
TexColor = texture(TreeTex, TexCoord).rgb;
}
else if(texID == 2)
{
TexColor = texture(SphinxTex, TexCoord).rgb;
}
else if(texID == 3)
{
TexColor = texture(SandTex, TexCoord).rgb;
}
else if(texID == 4)
{
TexColor = texture(PondTex, TexCoord).rgb;
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



//dot product for luminance
float luminance( vec3 color )
{
 return dot(lum,color);
}


vec4 pass1()
{
 return vec4(blinnPhongModel( Position, normalize(Normal) ),1.0);
}





//Computes vertical sum of pixels and applies them to texture
vec4 pass2()
{


//Mixes each pixel's colour with the sum of its surroundings

 ivec2 pix = ivec2( gl_FragCoord.xy );
 vec4 sum = texelFetch(BlurTex, pix, 0) * Weight[0];

 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,1) ) * Weight[1];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,-1) ) * Weight[1];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,2) ) * Weight[2];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,-2) ) * Weight[2];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,3) ) * Weight[3];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,-3) ) * Weight[3];
 //Pixels further away are given less weight, affecting the colour of the current pixel less
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,4) ) * Weight[4];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(0,-4) ) * Weight[4];

 return sum;


}

//Computes horizontal sum of pixels and applies them to texture
vec4 pass3()
{


 ivec2 pix = ivec2( gl_FragCoord.xy );
 vec4 sum = texelFetch(BlurTex, pix, 0) * Weight[0];

 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(1,0) ) * Weight[1];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(-1,0) ) * Weight[1];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(2,0) ) * Weight[2];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(-2,0) ) * Weight[2];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(3,0) ) * Weight[3];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(-3,0) ) * Weight[3];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(4,0) ) * Weight[4];
 sum += texelFetchOffset( BlurTex, pix, 0, ivec2(-4,0) ) * Weight[4];

 return sum;


}




void main() {


//Pass 1 renders scene image to a texture, pass 2 and 3 apply blur effect on the image
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
