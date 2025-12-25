#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Original;
uniform sampler2D u_Blurred;

out vec4 o_FragColor;

void main() {
    vec3 originalColor = texture(u_Original, a_TexCoord).rgb;
    vec3 blurredColor = texture(u_Blurred, a_TexCoord).rgb;
    vec3 color = mix(originalColor, blurredColor, 0.04);
    // vec3 color = originalColor + blurredColor;

    o_FragColor = vec4(color, 1.0);
}