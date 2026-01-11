#version 460 core

layout (location = 0) in vec3 a_Pos;

layout (location = 0) out vec3 o_LocalPos;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main() {
    o_LocalPos = a_Pos;
    gl_Position = u_Projection * u_View * vec4(a_Pos, 1.0);
}