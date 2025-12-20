#version 460 core

layout (location = 0) in vec3 a_LocalPos;

uniform samplerCube u_Skybox;
uniform float u_LOD;

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
    vec3 localPos = a_LocalPos;
    localPos.y *= -1.0;
    vec3 envColor = textureLod(u_Skybox, localPos, u_LOD).rgb;

    envColor = TonemapACES(envColor);
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    o_FragColor = vec4(envColor, 1.0);
    // o_FragColor = vec4(1.0);
}