#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout (location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
    if (fragTexCoord == vec2(0,0))
    {
        vec3 blending = abs(fragNormalWorld);
        blending = normalize(max(blending, 0.00001));
        float b = (blending.x + blending.y + blending.z);
        blending /= vec3(b, b, b);

        float scale = 1.f;
        vec4 xaxis = texture( texSampler, fragPosWorld.yz * scale);
        vec4 yaxis = texture( texSampler, fragPosWorld.xz * scale);
        vec4 zaxis = texture( texSampler, fragPosWorld.xy * scale);

        vec4 tex = xaxis * blending.x + yaxis * blending.y + zaxis * blending.z;

        outColor = tex;
    }
    else
        outColor = texture(texSampler, fragTexCoord);
}