#version 460 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in int a_MaterialIndex;  
layout (location = 4) in int a_ObjectIndex;

uniform mat4 u_ViewProjection;

layout(std430, binding = 0) buffer TransformBuffer {
    mat4 Transforms[];
};

layout (location = 0) out vec3 o_Normal;
layout (location = 1) out vec2 o_TexCoord;
layout (location = 2) out vec3 o_FragPos;
layout (location = 3) out flat int o_MaterialIndex;

void main() {
    gl_Position = u_ViewProjection * Transforms[a_ObjectIndex] * vec4(a_Pos, 1.0);

    o_Normal = mat3(transpose(inverse(Transforms[a_ObjectIndex]))) * a_Normal;
    o_TexCoord = a_TexCoord;
    o_FragPos = a_Pos;
    o_MaterialIndex = a_MaterialIndex;
}