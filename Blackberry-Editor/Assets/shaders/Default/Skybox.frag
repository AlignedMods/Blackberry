#version 460 core

layout (location = 0) in vec3 a_LocalPos;

uniform samplerCube u_Skybox;
uniform float u_LOD;

out vec4 o_FragColor;

void main() {
    vec3 localPos = normalize(a_LocalPos);
    vec3 envColor = textureLod(u_Skybox, localPos, u_LOD).rgb;
    // vec3 envColor = textureLod(u_Skybox, vec2(1.0), 0).rgb;
  
    // o_FragColor = vec4(normalize(a_LocalPos) * 0.5 + 0.5, 1.0);
    o_FragColor = vec4(envColor, 1.0);
}