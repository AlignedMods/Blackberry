#version 460 core
#extension GL_ARB_bindless_texture : enable

layout (location = 0) in vec2 a_TexCoord;

layout (std430, binding = 2) buffer GBuffer {
    uvec2 GPosition;
    uvec2 GNormal;
    uvec2 GAlbedo;
    uvec2 GMat;
};

struct PointLight {
    vec4 Position; // w is unused
    vec4 Color; // w is unused
    vec4 Params; // w is unused
};

layout (std430, binding = 3) buffer LightBuffer {
    PointLight Lights[];
};

uniform int u_LightCount;
uniform vec3 u_ViewPos;

out vec4 o_FragColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);

void main() {
    vec3 worldPos =   texture(sampler2D(GPosition), a_TexCoord).rgb;
    vec3 normal =     texture(sampler2D(GNormal), a_TexCoord).rgb;
    vec3 albedo = pow(texture(sampler2D(GAlbedo), a_TexCoord).rgb, vec3(2.2));
    float metallic =  texture(sampler2D(GMat), a_TexCoord).r;
    float roughness = texture(sampler2D(GMat), a_TexCoord).g;
    float ao =        texture(sampler2D(GMat), a_TexCoord).b;
    
    vec3 N = normal;
    vec3 V = normalize(u_ViewPos - worldPos);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_LightCount; i++) {
        vec3 position = Lights[i].Position.xyz;
        vec3 color = Lights[i].Color.rgb;

        float radius = Lights[i].Params.r;
        float intensity = Lights[i].Params.g;

        // calculate per light radiance
        vec3 L = normalize(position - worldPos);
        vec3 H = normalize(V + L);
        float distance = max(length(position - worldPos), 0.0);
        float x = clamp(1.0 - (distance / radius), 0.0, 1.0);
        float attenuation = x * x;
        vec3 radiance = color * attenuation * intensity;
        // radiance = vec3(1.0 / distance);
    
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
    
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
    
        // add to radiance
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        // Lo += radiance;
    }
    
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));
    
    o_FragColor = vec4(color, 1.0);
    // o_FragColor = vec4(vec3(length(u_ViewPos - worldPos) / 10.0), 1.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}