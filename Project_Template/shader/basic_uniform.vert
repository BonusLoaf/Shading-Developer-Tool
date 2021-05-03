#version 460

in vec3 VertexPosition;
in vec3 VertexNormal;
in vec2 VertexTexCoord;
out vec2 TexCoord;
out vec3 Vec;

out vec3 Position;
out vec3 Normal;

uniform mat4 ModelMatrix;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;


void main() { 



vec4 pos4 = vec4(VertexPosition, 1.0);

 Normal = normalize(NormalMatrix * VertexNormal);

 Position = vec3(pos4 * ModelViewMatrix);

 gl_Position = MVP * vec4(VertexPosition,1.0);


//Vertex position for normal map
Vec = VertexPosition;

//Texture coordinates for setting texture
TexCoord = VertexTexCoord;
 
}