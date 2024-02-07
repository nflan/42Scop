#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.f, -1.f),
  vec2(-1.f, 1.f),
  vec2(1.f, -1.f),
  vec2(1.f, -1.f),
  vec2(-1.f, 1.f),
  vec2(1.f, 1.f)
);

layout (location = 0) out vec2 fragOffset;

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
    PointLight pointLights[1]; // change to modify lights
    float ni; // optical density
    float ns; // shininess
    float d;  // transparency
    int illum; //illum = 1 a flat material with no specular highlights, illum = 2 denotes the presence of specular highlights
    int numLights;
} ubo;

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

void main()
{
  fragOffset = OFFSETS[gl_VertexIndex];
  vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
  vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

  vec3 positionWorld = push.position.xyz
    + push.radius * fragOffset.x * cameraRightWorld
    + push.radius * fragOffset.y * cameraUpWorld;

  gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.f);
}
