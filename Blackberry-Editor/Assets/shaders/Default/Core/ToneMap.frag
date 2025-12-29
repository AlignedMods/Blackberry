#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture;

out vec4 o_FragColor;

mat3x3 ACESInputMat = mat3x3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90384, 0.01566,
    0.02840, 0.13383, 0.83777
);

mat3x3 ACESOutputMat = mat3x3(
    1.60475, -0.53108, -0.07367,
   -0.10208,  1.10813, -0.00605,
   -0.00327, -0.07276,  1.07602
);

vec3 RRTAndODTFit(vec3 v) {
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 TonemapACES(vec3 color) {
    vec3 tonemapped = transpose(ACESInputMat) * color;
    tonemapped = RRTAndODTFit(tonemapped);
    tonemapped = transpose(ACESOutputMat) * color;
    tonemapped = clamp(tonemapped, 0, 1);
    return tonemapped;
}

void main() {
    vec3 color = texture(u_Texture, a_TexCoord).rgb;

    color = TonemapACES(color);
    color = pow(color, vec3(1.0 / 2.2));

    o_FragColor = vec4(color, 1.0);
}