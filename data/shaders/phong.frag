#version 460 core

layout (location = 0) out vec4 FragColor;

flat in float TexId;
flat in vec2 TexOffset;
flat in vec2 TexSize;
in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;
in vec3 diffuse;
in vec3 specular;
in vec3 ambient;

uniform vec3 cameraPos;
//uniform sampler2D tex[1];

const int POINT_LIGHT_COUNT = ${POINT_LIGHT_COUNT};
const float PI = 3.1415926536f;
const float INV_PI = 0.318309886183790671537767526745028724f;

struct PointLight {
    vec3 Position;
    vec3 Color;
};

uniform PointLight pointLights[POINT_LIGHT_COUNT];
uniform uint TEXTURE_MAX_SIZE;
uniform sampler2DArray textures;

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightVec = light.Position - fragPos;
    vec3 lightDir = normalize(lightVec);
    float distance = length(lightVec);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.f);
    vec3 diffuseLight = light.Color * diff;
    diffuseLight *= 1.f / (distance * distance);
    return diffuseLight;
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

    if (TexId >= 0) {
        // FIXME: only accept [0, 1] coordinates
        vec2 Coord = (fract(TexCoord) * TexSize + TexOffset) / TEXTURE_MAX_SIZE;
        vec3 arrayCoord = vec3(Coord, TexId);
        // FIXME: texture function is not working, except we use constant coordinates
        diffuseResult = texture(textures, arrayCoord).rgb;
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
//    FragColor = vec4(diffuse, 1.f);
//    FragColor = vec4(TexCoord, 1.f, 1.f);
}
