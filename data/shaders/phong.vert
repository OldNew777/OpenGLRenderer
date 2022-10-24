#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aDiffuse;
layout (location = 3) in vec3 aSpecular;
layout (location = 4) in vec3 aAmbient;
layout (location = 5) in vec3 aTexCoords;
layout (location = 6) flat in sampler2D aDiffuseTexHandle;

out float DiffuseTex;
out vec2 DiffuseTexCoord;
out vec3 Position;
out vec3 Normal;
out vec3 Diffuse;
out vec3 Specular;
out vec3 Ambient;
flat out sampler2D DiffuseTexHandle;

uniform mat4 view;
uniform mat4 projection;

const float PI = 3.1415926536f;

void main() {
    Position = aPos;
    DiffuseTexCoord = aTexCoords.xy;
    DiffuseTex = aTexCoords.z;

    Normal = aNormal;

    Diffuse = aDiffuse;
    Specular = aSpecular;
    Ambient = aAmbient;
    DiffuseTexHandle = aDiffuseTexHandle;

    gl_Position = projection * view * vec4(aPos, 1.0f);
//    gl_Position /= gl_Position.w;
}
