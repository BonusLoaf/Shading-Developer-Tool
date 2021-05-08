#version 460


in vec4 Position;
in vec3 Normal;

uniform struct lightInfo
{
vec4 Position;
vec3 La;
vec3 L;
}lights;

uniform struct materialInfo
{
vec3 Ka;
vec3 Kd;
vec3 Ks;
float Shininess;
}Material;

layout (location = 0) out vec4 FragColor;

in vec2 TexCoord;


vec3 blinnPhongModel(vec3 position, vec3 normal)
{


//Ambient
vec3 ambient = lights.La * lights.L;

//Diffuse
vec3 s = normalize(vec3(lights.Position) - position);

float sDotN = dot(s,normal);

vec3 diffuse = lights.L * sDotN;

//Specular
vec3 specular = Material.Ks * lights.L * sDotN;

vec3 v = normalize(-position.xyz);




//Shininess
if(sDotN > 0.0)
{

//Half Vector
vec3 h = normalize(v+s);


//Calculate specular with Shininess
specular = Material.Ks * pow(max(dot(h,normal),0.0), Material.Shininess);

}

return ambient + lights.L * (diffuse + specular);



}


//Same as basic(gauss) uniform
void main() {

    FragColor = vec4 (blinnPhongModel(vec3(Position), normalize(Normal)),1.0);

}
