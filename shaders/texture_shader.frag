#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

struct PointLight {
    vec4 position; // ignore w
    vec4 color; // w is intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec3 ka; // ambient color
    vec3 kd; // diffuse color
    vec3 ks; // specular color
    vec3 ke; // emissive color
    int illum; //illum = 1 a flat material with no specular highlights, illum = 2 denotes the presence of specular highlights
    float ns; // shininess
    float ni; // optical density
    float d;  // transparency
    PointLight pointLights[2];
    int numLights;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}