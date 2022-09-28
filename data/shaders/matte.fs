#version 410 core

layout (location = 0) out vec4 FragColor;

flat in float TexId;
flat in vec2 TexOffset;
flat in vec2 TexSize;
in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 r;
in float Sigma;
in float a;
in float b;

uniform vec3 cameraPos;

const int POINT_LIGHT_COUNT = ${POINT_LIGHT_COUNT};
const float PI = 3.1415926536f;
const float INV_PI = 0.318309886183790671537767526745028724f;

struct PointLight {
    vec3 Position;
    vec3 Color;
};

uniform PointLight pointLights[POINT_LIGHT_COUNT];

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.Position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0f);
    vec3 diffuse = light.Color * diff;
    return diffuse;
}

void main()
{
    vec3 viewDir = normalize(cameraPos - Position);
    vec3 norm = normalize(dot(Normal, viewDir) >= 0.0f ? Normal : -Normal);

    vec3 Lo = vec3(0.0f);

//     // phase 1: directional light
//     vec3 result = CalcDirLight(dirLight, norm, viewDir);
    // phase 2: point lights
    for(int i = 0; i < POINT_LIGHT_COUNT; i++) {
        vec3 lightDir = normalize(pointLights[i].Position - Position);
        bool valid = (viewDir.z * lightDir.z) > 0.0f;
        if (!valid) {
            continue;
        }
        vec3 f = r * INV_PI;
        if (Sigma > 0.0f) {
            // TODO
        }
        Lo += CalcPointLight(pointLights[i], norm, Position, viewDir) * f;
    }
//     // phase 3: spot light
//     result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(Lo, 1.0f);
}
