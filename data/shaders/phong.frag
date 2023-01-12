#version 460 core

layout (location = 0) out vec4 FragColor;

in float DiffuseTex;
in vec2 DiffuseTexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 diffuse;
in vec3 specular;
in vec3 ambient;

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
    samplerCube ShadowCubeMap;
    float FarPlane;
};
const float SHADOW_BIAS = 0.07f;

uniform PointLight pointLights[POINT_LIGHT_COUNT];
uniform sampler2D textures[${TEXTURE_COUNT}];
uniform bool enable_shadow;

// calculates the color when using a point light.
vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
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

float CalculateShadow(PointLight light, vec3 fragPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - light.Position;
    // ise the fragment to light vector to sample from the depth map
    float closestDepth = texture(light.ShadowCubeMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= light.FarPlane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float shadow = currentDepth - SHADOW_BIAS > closestDepth ? 1.0 : 0.0;
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;
}

bool same_hemisphere(vec3 v1, vec3 v2, vec3 normal) {
    return dot(v1, normal) > 0.f && dot(v2, normal) > 0.f;
}

void main()
{
    vec3 viewDir = normalize(cameraPos - Position);
    vec3 norm = normalize(Normal);

    vec3 Lo = vec3(0.f);
    vec3 diffuseResult = diffuse;

    if (DiffuseTex >= 0.f) {
        vec2 Coord = fract(DiffuseTexCoord);
        diffuseResult = texture(textures[0], Coord).rgb;
    }

    // point lights
    for(int i = 0; i < POINT_LIGHT_COUNT; i++) {
        vec3 lightDir = normalize(pointLights[i].Position - Position);
        bool valid = same_hemisphere(lightDir, viewDir, norm);
        if (!valid) {
            continue;
        }
        float shadow = enable_shadow ? CalculateShadow(pointLights[i], Position) : 0.f;
        vec3 lightColor = CalculatePointLight(pointLights[i], norm, Position, viewDir);
        Lo += (1.f - shadow) * lightColor * diffuseResult;
    }

//    FragColor = vec4(Lo, 1.f);
//    FragColor = vec4(norm * 0.5f + 0.5f, 1.f);
    FragColor = vec4(diffuseResult, 1.f);
//    FragColor = vec4(TexCoord, 1.f, 1.f);
//    float cameraDistance = length(cameraPos - Position) / 20.f;
//    FragColor = vec4(vec3(cameraDistance), 1.f);
}
