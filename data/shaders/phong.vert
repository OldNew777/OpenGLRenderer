#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aDiffuse;
layout (location = 3) in vec3 aTexCoords;
layout (location = 4) in vec4 aTexProperty;
layout (location = 5) in vec3 aSpecular;
layout (location = 6) in vec3 aAmbient;

flat out float TexId;
flat out vec2 TexOffset;
flat out vec2 TexSize;
out vec2 TexCoord;
out vec3 Position;
out vec3 Normal;
out vec3 diffuse;
out vec3 specular;
out vec3 ambient;

uniform mat4 view;
uniform mat4 projection;
uniform sampler2DArray textures;

const float PI = 3.1415926536f;

void main() {
    Position = aPos;
    TexCoord = aTexCoords.xy;
    TexId = aTexCoords.z;
    TexOffset = aTexProperty.xy;
    TexSize = aTexProperty.zw;

    Normal = aNormal;

    if (TexId < 0) {
        diffuse = aDiffuse;
    } else {
        vec2 Coord = (fract(fract(TexCoord) + 1.0f) * TexSize + TexOffset) / ${TEXTURE_MAX_SIZE};
        //vec2 Coord = TexOffset;
        diffuse = texture(textures, vec3(Coord, TexId)).rgb;
    }
    specular = aSpecular;
    ambient = aAmbient;

    gl_Position = projection * view * vec4(aPos, 1.0f);
//    gl_Position /= gl_Position.w;
}
