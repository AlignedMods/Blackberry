#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture;
uniform int u_Horizontal;

const float WEIGHT[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

out vec4 o_FragColor;

void main() {
    vec2 texOffset = 1.0 / textureSize(u_Texture, 0);
    vec3 result = texture(u_Texture, a_TexCoord).rgb * WEIGHT[0];

    if (u_Horizontal == 0) { // If horizontal is 0 then that means that we DO actually do a horizontal blur, weird
        for (int i = 0; i < 5; i++) {
            result += texture(u_Texture, a_TexCoord + vec2(texOffset.x * i, 0.0)).rgb * WEIGHT[i];
            result += texture(u_Texture, a_TexCoord - vec2(texOffset.x * i, 0.0)).rgb * WEIGHT[i];
        }
    } else {
        for (int i = 0; i < 5; i++) {
            result += texture(u_Texture, a_TexCoord + vec2(0.0, texOffset.y * i)).rgb * WEIGHT[i];
            result += texture(u_Texture, a_TexCoord - vec2(0.0, texOffset.y * i)).rgb * WEIGHT[i];
        }
    }

    o_FragColor = vec4(result, 1.0);
}