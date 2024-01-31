#version 450

layout (location = 0) in vec2 fragOffset;
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

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

const float M_PI = 3.1415926538;

void main() {
  float dis = sqrt(dot(fragOffset, fragOffset));
  if (dis >= 1.f) {
    discard;
  }

  float cosDis = 0.5f * (cos(dis * M_PI) + 1.f); // ranges from 1 -> 0
  outColor = vec4(push.color.xyz + 0.5f * cosDis, cosDis);
}