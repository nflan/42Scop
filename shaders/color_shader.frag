#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;

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

layout (binding = 1) uniform UboInstance {
	mat4 model; 
} uboInstance;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main()
{
    vec3 diffuseLight = ubo.ka * ubo.ni;
    vec3 specularLight = ubo.ks;
    vec3 surfaceNormal = normalize(fragNormalWorld);

    vec3 cameraPosWorld = ubo.invView[3].xyz;
    vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

    for (int i = 0; i < ubo.numLights; i++) {
        PointLight light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = dot(directionToLight, directionToLight);
        directionToLight = normalize(directionToLight);

        float cosAngIncidence = max(dot(surfaceNormal, directionToLight), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuseLight += intensity * cosAngIncidence;

        vec3 halfAngle = normalize(directionToLight + viewDirection);
        float blinnTerm = dot(surfaceNormal, halfAngle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, ubo.ns);
        specularLight += intensity * blinnTerm;
    }
    
    vec3 finalDiffuseColor = ubo.kd * fragColor;
    vec3 finalSpecularColor = ubo.ks * fragColor;

    if (ubo.illum != 1)
        outColor = vec4((diffuseLight * finalDiffuseColor + specularLight * finalSpecularColor) + ubo.ke, ubo.d);
    else
        outColor = vec4((diffuseLight * finalDiffuseColor) + ubo.ke, ubo.d);

}