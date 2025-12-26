#version 460 core

layout (location = 0) in vec2 a_TexCoord;

uniform sampler2D u_Texture;
uniform vec2 u_TexResolution;
uniform int u_CurrentMip;

out vec4 o_FragColor;

vec3 PowVec3(vec3 v, float p) {
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float INV_GAMMA = 1.0 / 2.2;
vec3 TosRGB(vec3 v) { return PowVec3(v, INV_GAMMA); }

float sRGBToLuma(vec3 col) {
    return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(vec3 col) {
    float luma = sRGBToLuma(TosRGB(col)) * 0.25;
    return 1.0 / (1.0 + luma);
}

void main() {
    vec2 srcTexelSize = 1.0 / u_TexResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    vec3 a = texture(u_Texture, vec2(a_TexCoord.x - 2*x, a_TexCoord.y + 2*y)).rgb;
    vec3 b = texture(u_Texture, vec2(a_TexCoord.x,       a_TexCoord.y + 2*y)).rgb;
    vec3 c = texture(u_Texture, vec2(a_TexCoord.x + 2*x, a_TexCoord.y + 2*y)).rgb;
                                     
    vec3 d = texture(u_Texture, vec2(a_TexCoord.x - 2*x, a_TexCoord.y)).rgb;
    vec3 e = texture(u_Texture, vec2(a_TexCoord.x,       a_TexCoord.y)).rgb;
    vec3 f = texture(u_Texture, vec2(a_TexCoord.x + 2*x, a_TexCoord.y)).rgb;
                                     
    vec3 g = texture(u_Texture, vec2(a_TexCoord.x - 2*x, a_TexCoord.y - 2*y)).rgb;
    vec3 h = texture(u_Texture, vec2(a_TexCoord.x,       a_TexCoord.y - 2*y)).rgb;
    vec3 i = texture(u_Texture, vec2(a_TexCoord.x + 2*x, a_TexCoord.y - 2*y)).rgb;
                                     
    vec3 j = texture(u_Texture, vec2(a_TexCoord.x - x, a_TexCoord.y + y)).rgb;
    vec3 k = texture(u_Texture, vec2(a_TexCoord.x + x, a_TexCoord.y + y)).rgb;
    vec3 l = texture(u_Texture, vec2(a_TexCoord.x - x, a_TexCoord.y - y)).rgb;
    vec3 m = texture(u_Texture, vec2(a_TexCoord.x + x, a_TexCoord.y - y)).rgb;

    vec3 groups[5];
    switch (u_CurrentMip) {
        case 0: 
            groups[0] = (a+b+d+e) * (0.125f/4.0f);
	        groups[1] = (b+c+e+f) * (0.125f/4.0f);
	        groups[2] = (d+e+g+h) * (0.125f/4.0f);
	        groups[3] = (e+f+h+i) * (0.125f/4.0f);
	        groups[4] = (j+k+l+m) * (0.5f/4.0f);
	        groups[0] *= KarisAverage(groups[0]);
	        groups[1] *= KarisAverage(groups[1]);
	        groups[2] *= KarisAverage(groups[2]);
	        groups[3] *= KarisAverage(groups[3]);
	        groups[4] *= KarisAverage(groups[4]);
	        o_FragColor = vec4(groups[0]+groups[1]+groups[2]+groups[3]+groups[4], 1.0);
	        o_FragColor = vec4(max(o_FragColor.rgb, 0.0001f), 1.0);
	        break;
        default:
            o_FragColor = vec4(e * 0.125, 1.0);
            o_FragColor += vec4((a + c + g + i) * 0.03125, 1.0);
            o_FragColor += vec4((b + d + f + h) * 0.0625, 1.0);
            o_FragColor += vec4((j + k + l + m) * 0.125, 1.0);
            break;
    }
}