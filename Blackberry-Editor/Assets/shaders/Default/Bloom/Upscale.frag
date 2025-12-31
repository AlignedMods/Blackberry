#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture0; // The lower resolution texture which will be upscaled
uniform sampler2D u_Texture1; // The current resolution downsample texture
uniform float u_FilterRadius;

out vec4 o_FragColor;

void main() {
    // float x = u_FilterRadius;
    // float y = u_FilterRadius;
    // 
    // vec3 a = texture(u_Texture0, vec2(a_TexCoord.x - x, a_TexCoord.y + y)).rgb;
    // vec3 b = texture(u_Texture0, vec2(a_TexCoord.x,     a_TexCoord.y + y)).rgb;
    // vec3 c = texture(u_Texture0, vec2(a_TexCoord.x + x, a_TexCoord.y + y)).rgb;
    //                                                    
    // vec3 d = texture(u_Texture0, vec2(a_TexCoord.x - x, a_TexCoord.y)).rgb;
    // vec3 e = texture(u_Texture0, vec2(a_TexCoord.x,     a_TexCoord.y)).rgb;
    // vec3 f = texture(u_Texture0, vec2(a_TexCoord.x + x, a_TexCoord.y)).rgb;
    //                                                    
    // vec3 g = texture(u_Texture0, vec2(a_TexCoord.x - x, a_TexCoord.y - y)).rgb;
    // vec3 h = texture(u_Texture0, vec2(a_TexCoord.x,     a_TexCoord.y - y)).rgb;
    // vec3 i = texture(u_Texture0, vec2(a_TexCoord.x + x, a_TexCoord.y - y)).rgb;
    // 
    // vec3 finalColor = e * 4.0;
    // finalColor += (b + d + f + h) * 2.0;
    // finalColor += (a + c + g + i);
    // finalColor *= 1.0 / 16.0;

    vec3 finalColor = texture(u_Texture0, a_TexCoord).rgb;

    o_FragColor = vec4(texture(u_Texture1, a_TexCoord).rgb + finalColor, 1.0);
}