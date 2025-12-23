#version 460 core
#extension GL_ARB_bindless_texture : enable // enable ARB bindless textures

layout (location = 0) in vec3 a_Normal;
layout (location = 1) in vec2 a_TexCoord;
layout (location = 2) in vec3 a_FragPos;
layout (location = 3) in flat int a_MaterialIndex;
layout (location = 4) in flat int a_EntityID;

struct Material {
    int UseAlbedoTexture;
    uvec2 AlbedoTexture;
    vec4 AlbedoColor;

    int UseMetallicTexture;
    uvec2 MetallicTexture;
    float MetallicFactor;

    int UseRoughnessTexture;
    uvec2 RoughnessTexture;
    float RoughnessFactor;

    int UseAOTexture;
    uvec2 AOTexture;
    float AOFactor;
};

layout (std430, binding = 1) buffer MaterialBuffer {
    Material Materials[];
};

layout (location = 0) out vec4 o_GPosition;
layout (location = 1) out vec4 o_GNormal;
layout (location = 2) out vec4 o_GAlbedo;
layout (location = 3) out vec4 o_GMat;
layout (location = 4) out int o_GEntityID;
 
void main() {
    // Store the position in the first buffer
    o_GPosition.rgb = a_FragPos;
    // Store the normal in the second buffer
    o_GNormal.rgb = normalize(a_Normal);
    // Store the albedo color in the third buffer
    if (Materials[a_MaterialIndex].UseAlbedoTexture == 1) {
        o_GAlbedo.rgb = texture(sampler2D(Materials[a_MaterialIndex].AlbedoTexture), a_TexCoord).rgb;
    } else {
        o_GAlbedo.rgb = Materials[a_MaterialIndex].AlbedoColor.rgb;
    }
    // Store material information in the fourth buffer
    if (Materials[a_MaterialIndex].UseMetallicTexture == 1) {
        o_GMat.r = texture(sampler2D(Materials[a_MaterialIndex].MetallicTexture), a_TexCoord).r;
    } else {
        o_GMat.r = Materials[a_MaterialIndex].MetallicFactor;
    }
    
    if (Materials[a_MaterialIndex].UseRoughnessTexture == 1) {
        o_GMat.g = texture(sampler2D(Materials[a_MaterialIndex].RoughnessTexture), a_TexCoord).r;
    } else {
        o_GMat.g = Materials[a_MaterialIndex].RoughnessFactor;
    }
    
    if (Materials[a_MaterialIndex].UseAOTexture == 1) {
        o_GMat.b = texture(sampler2D(Materials[a_MaterialIndex].AOTexture), a_TexCoord).r;
    } else {
        o_GMat.b = Materials[a_MaterialIndex].AOFactor;
    }

    o_GMat.a = float(a_EntityID);

    // For visualizations (normally the alpha would just get set to 0.0)
    o_GPosition.a = 1.0;
    o_GNormal.a = 1.0;
    o_GAlbedo.a = 1.0;
    // o_GMat.a = 1.0;
    // Mouse picking
    o_GEntityID = a_EntityID;
}