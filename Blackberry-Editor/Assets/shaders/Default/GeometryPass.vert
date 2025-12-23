#version 460 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in int a_MaterialIndex;  
layout (location = 4) in int a_ObjectIndex;
layout (location = 5) in int a_EntityID;

uniform mat4 u_ViewProjection;

layout(std430, binding = 0) buffer TransformBuffer {
    mat4 Transforms[];
};

layout (location = 0) out vec3 o_Normal;
layout (location = 1) out vec2 o_TexCoord;
layout (location = 2) out vec3 o_FragPos;
layout (location = 3) out flat int o_MaterialIndex;
layout (location = 4) out flat int o_EntityID;

void main() {
    vec4 worldPos = Transforms[a_ObjectIndex] * vec4(a_Pos, 1.0);

    gl_Position = u_ViewProjection * worldPos;

    o_Normal = mat3(transpose(inverse(Transforms[a_ObjectIndex]))) * a_Normal;
    o_TexCoord = a_TexCoord;
    o_FragPos = worldPos.xyz;
    o_MaterialIndex = a_MaterialIndex;
    o_EntityID = a_EntityID;
}