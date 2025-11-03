#version 460 core

layout (location = 0) in vec2 a_Pos;      // full-screen quad positions
layout (location = 1) in vec2 a_TexCoord; // texture coordinates

uniform mat4 u_Projection;

layout (location = 0) out vec2 o_TexCoord;

void main()
{
    gl_Position = vec4(a_Pos, 0.0, 1.0);
    o_TexCoord = a_TexCoord;
}