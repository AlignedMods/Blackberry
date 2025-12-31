#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_PBROutput;
uniform float u_Threshold;

out vec4 o_FragColor;

void main() {
    vec3 tex = texture(u_PBROutput, a_TexCoord).rgb;

    vec3 color = max(tex - vec3(u_Threshold), vec3(0.0));

    float brightness = dot(tex, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > u_Threshold) {
        o_FragColor = vec4(tex, 1.0);
    } else {
        o_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }

    o_FragColor = vec4(color, 1.0);
}