#version 460 core

layout (location = 0) in vec3 a_LocalPos;

uniform samplerCube u_Skybox;

out vec4 o_FragColor;

void main() {
    vec3 localPos = a_LocalPos;
    localPos.y *= -1.0;
    vec3 envColor = texture(u_Skybox, localPos).rgb;

    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    o_FragColor = vec4(envColor, 1.0);
    // o_FragColor = vec4(1.0);
}