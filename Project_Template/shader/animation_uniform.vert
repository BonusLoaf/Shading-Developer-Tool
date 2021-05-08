#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec4 Position;
out vec3 Normal;
out vec2 TexCoord;

uniform float Time;

uniform float Freq = 2.0;
uniform float Velocity = 2.0;
uniform float Amp = 0.8;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;


//Sets all parameters necessary for frag uniform
void main() { 

vec4 pos4 = vec4(VertexPosition, 1.0);

//transforms vertices on y coordinates using 'time' variable
float u = Freq * pos4.x - Velocity * Time;

pos4.y = Amp * sin(u);

//calc normal vector
vec3 n = vec3(0.0);

n.xy = normalize(vec2(cos(u), 1.0));


Position = ModelViewMatrix * pos4;

Normal = NormalMatrix * n;
TexCoord = VertexTexCoord;

gl_Position = MVP * pos4;


}
