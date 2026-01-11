#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture;

out vec4 o_FragColor;

void main() {
    float entity = texture(u_Texture, a_TexCoord).r;

    if (entity != -1.0)
        o_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}