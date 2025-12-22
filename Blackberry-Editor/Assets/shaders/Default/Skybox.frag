#version 460 core

layout (location = 0) in vec3 a_LocalPos;

uniform samplerCube u_Skybox;
uniform float u_LOD;

out vec4 o_FragColor;

void main() {
    vec3 localPos = a_LocalPos;
    localPos.y *= -1.0;
    vec3 envColor = textureLod(u_Skybox, localPos, u_LOD).rgb;
  
    o_FragColor = vec4(envColor, 1.0);
}