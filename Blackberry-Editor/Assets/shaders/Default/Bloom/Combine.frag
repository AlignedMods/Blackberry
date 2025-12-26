#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Original;
uniform sampler2D u_Blurred;

uniform float u_CombineAmount;
uniform int u_Mode;

out vec4 o_FragColor;

void main() {
    vec3 originalColor = texture(u_Original, a_TexCoord).rgb;
    vec3 blurredColor = texture(u_Blurred, a_TexCoord).rgb;

    vec3 color;
    switch (u_Mode) {
        // Additive blending
        case 0: color = originalColor + blurredColor; break;
        // Linear blending
        case 1: color = mix(originalColor, blurredColor, u_CombineAmount); break;
        default: color = originalColor;
    }

    o_FragColor = vec4(color, 1.0);
}