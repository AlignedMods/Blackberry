#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture;
uniform float u_FilterRadius;

out vec4 o_FragColor;

void main() {
    float x = u_FilterRadius;
    float y = u_FilterRadius;

    vec3 a = texture(u_Texture, vec2(a_TexCoord.x - x, a_TexCoord.y + y)).rgb;
    vec3 b = texture(u_Texture, vec2(a_TexCoord.x,     a_TexCoord.y + y)).rgb;
    vec3 c = texture(u_Texture, vec2(a_TexCoord.x + x, a_TexCoord.y + y)).rgb;
                                                       
    vec3 d = texture(u_Texture, vec2(a_TexCoord.x - x, a_TexCoord.y)).rgb;
    vec3 e = texture(u_Texture, vec2(a_TexCoord.x,     a_TexCoord.y)).rgb;
    vec3 f = texture(u_Texture, vec2(a_TexCoord.x + x, a_TexCoord.y)).rgb;
                                                       
    vec3 g = texture(u_Texture, vec2(a_TexCoord.x - x, a_TexCoord.y - y)).rgb;
    vec3 h = texture(u_Texture, vec2(a_TexCoord.x,     a_TexCoord.y - y)).rgb;
    vec3 i = texture(u_Texture, vec2(a_TexCoord.x + x, a_TexCoord.y - y)).rgb;

    o_FragColor = vec4(e * 4.0, 1.0);
    o_FragColor += vec4((b+d+f+h)*2.0, 1.0);
    o_FragColor += vec4((a+c+g+i), 1.0);
    o_FragColor *= vec4(1.0 / 16.0);
}