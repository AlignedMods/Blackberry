#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture;

out vec4 o_FragColor;

vec3 TonemapACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
    vec3 color = texture(u_Texture, a_TexCoord).rgb;

    color = TonemapACES(color);
    color = pow(color, vec3(1.0 / 2.2));

    o_FragColor = vec4(color, 1.0);
}