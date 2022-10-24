#version 460 core

layout (location = 0) out vec4 FragColor;

in float DiffuseTex;
in vec2 DiffuseTexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 Diffuse;
in vec3 Specular;
in vec3 Ambient;
flat in sampler2D DiffuseTexHandle;

uniform vec3 cameraPos;

const int POINT_LIGHT_COUNT = ${POINT_LIGHT_COUNT};
const float PI = 3.1415926536f;
const float INV_PI = 0.318309886183790671537767526745028724f;

struct PointLightFactor {
    float constant;
    float linear;
    float quadratic;
};
const PointLightFactor POINT_LIGHT_FACTOR = {0.9f, 0.5f, 1.f};

struct PointLight {
    vec3 Position;
    vec3 Color;
    sampler2D ShadowMap;
};

uniform PointLight pointLights[POINT_LIGHT_COUNT];

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightVec = light.Position - fragPos;
    vec3 lightDir = normalize(lightVec);
    float distance = length(lightVec);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.f);
    vec3 diffuseLight = light.Color * diff;
    float attenuation = 1.f / (POINT_LIGHT_FACTOR.constant + POINT_LIGHT_FACTOR.linear * distance +
        POINT_LIGHT_FACTOR.quadratic * (distance * distance));
    return diffuseLight * attenuation;
}

bool same_hemisphere(vec3 v1, vec3 v2, vec3 normal) {
    return dot(v1, normal) > 0.f && dot(v2, normal) > 0.f;
}

void main()
{
    vec3 viewDir = normalize(cameraPos - Position);
    vec3 norm = normalize(Normal);

    vec3 Lo = vec3(0.f);
    vec3 diffuseResult = Diffuse;

    if (DiffuseTex >= 0.f) {
        vec2 Coord = fract(DiffuseTexCoord);
        diffuseResult = texture(DiffuseTexHandle, Coord).rgb;
    }

    // point lights
    for(int i = 0; i < POINT_LIGHT_COUNT; i++) {
        vec3 lightDir = normalize(pointLights[i].Position - Position);
        bool valid = same_hemisphere(lightDir, viewDir, norm);
        if (!valid) {
            continue;
        }
        Lo += CalcPointLight(pointLights[i], norm, Position, viewDir) * diffuseResult;
    }

    FragColor = vec4(Lo, 1.f);
//    FragColor = vec4(norm * 0.5f + 0.5f, 1.f);
//    FragColor = vec4(diffuseResult, 1.f);
//    FragColor = vec4(TexCoord, 1.f, 1.f);
}
