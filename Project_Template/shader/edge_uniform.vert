#version 460

in vec3 VertexPosition;
in vec3 VertexNormal;

uniform mat4 ModelViewMatrix;
uniform mat4 ModelMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;


out vec3 Position;
out vec3 Normal;

//Sets all parameters necessary for frag uniform
void main() { 
 
 vec4 pos4 = vec4(VertexPosition, 1.0);

 Normal = normalize(NormalMatrix * VertexNormal);

 Position = vec3(pos4 * ModelViewMatrix);

 gl_Position = MVP * vec4(VertexPosition,1.0);

}
