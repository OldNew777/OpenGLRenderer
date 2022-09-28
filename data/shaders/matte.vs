#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 kd;
layout (location = 3) in vec3 aTexCoords;
layout (location = 4) in vec4 aTexProperty;
layout (location = 5) in float sigma;

flat out float TexId;
flat out vec2 TexOffset;
flat out vec2 TexSize;
out vec2 TexCoord;
out vec3 Position;
out vec3 Normal;
out vec3 r;
out float Sigma;
out float a;
out float b;

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

    vec4 PosInView = view * vec4(aPos, 1.0f);

    Normal = aNormal;

    r = kd;
    float sigma2 = sigma * PI / 180.0f;
    sigma2 *= sigma2;
    a = 1.f - sigma2 / (2.0f * sigma2 + 0.66f);
    b = 0.45f * sigma2 / (sigma2 + 0.09f);

    gl_Position = projection * PosInView;
}
