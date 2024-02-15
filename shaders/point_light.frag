#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec3 ka;
    vec3 kd;
    vec3 ks;
    vec3 ke;
    PointLight pointLights[1];
    float ni;
    float ns;
    float d;
    int illum;
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

  float cosDis = 0.5f * (cos(dis * M_PI) + 1.f);
  outColor = vec4(push.color.xyz + 0.5f * cosDis, cosDis);
}