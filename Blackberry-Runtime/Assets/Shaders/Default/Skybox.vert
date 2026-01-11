#version 460 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;

layout (location = 0) out vec3 o_LocalPos;

void main() {
    o_LocalPos = a_Pos;

    mat4 rotView = mat4(mat3(u_View));
    vec4 clipPos = u_Projection * rotView * vec4(a_Pos, 1.0);

    gl_Position = clipPos.xyww;
}