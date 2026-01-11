#version 460 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;

// Data about a specific instance
struct InstanceData {
    mat4 Transform;
    int MaterialIndex;
    int EntityID;
};

layout(std430, binding = 0) buffer InstanceDataBuffer {
    InstanceData Instances[];
};

layout (location = 0) out vec3 o_Normal;
layout (location = 1) out vec2 o_TexCoord;
layout (location = 2) out vec3 o_FragPos;
layout (location = 3) out flat int o_MaterialIndex;
layout (location = 4) out flat int o_EntityID;

void main() {
    vec4 worldPos = Instances[gl_InstanceID].Transform * vec4(a_Pos, 1.0);

    gl_Position = u_ViewProjection * worldPos;

    mat3 normalMatrix = transpose(inverse(mat3(Instances[gl_InstanceID].Transform)));
    o_Normal = normalMatrix * a_Normal;

    o_TexCoord = a_TexCoord;
    o_FragPos = worldPos.xyz;
    o_MaterialIndex = Instances[gl_InstanceID].MaterialIndex;
    o_EntityID = Instances[gl_InstanceID].EntityID;
}