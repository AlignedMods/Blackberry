#version 460 core
#extension GL_ARB_bindless_texture : enable

layout (location = 0) in vec2 a_TexCoord;

layout (std430, binding = 2) buffer GBuffer {
    uvec2 GPosition;
    uvec2 GNormal;
    uvec2 GAlbedo;
    uvec2 GMat;
};

struct DirectionalLight {
    vec4 Direction; // w is unused
    vec4 Color; // w is unused
    vec4 Params; // g, b, w is unused
};

struct PointLight {
    vec4 Position; // w is unused
    vec4 Color; // w is unused
    vec4 Params; // w is unused
};

struct SpotLight {
    vec4 Position; // w is unused
    vec4 Direction; // w is used for cutoff
    vec4 Color; // w is used for intensity
};

layout (std430, binding = 3) buffer PointLightBuffer {
    PointLight PointLights[];
};

layout (std430, binding = 4) buffer SpotLightBuffer {
    SpotLight SpotLights[];
};

uniform int u_PointLightCount;
uniform int u_SpotLightCount;
uniform DirectionalLight u_DirectionalLight;

uniform vec3 u_ViewPos;

uniform samplerCube u_IrradianceMap;
uniform samplerCube u_PrefilterMap;
uniform sampler2D u_BrdfLUT;

uniform vec3 u_FogColor;
uniform float u_FogDistance;

out vec4 o_FragColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 F0);
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);

vec3 AddLight(vec3 N, vec3 H, vec3 V, vec3 L, vec3 F0, float roughness, float metallic, vec3 albedo, vec3 radiance) {
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
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

vec3 TonemapACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
    vec3 worldPos =   texture(sampler2D(GPosition), a_TexCoord).rgb;
    vec3 normal =     texture(sampler2D(GNormal), a_TexCoord).rgb;
    vec3 albedo = pow(texture(sampler2D(GAlbedo), a_TexCoord).rgb, vec3(2.2));
    float metallic =  texture(sampler2D(GMat), a_TexCoord).r;
    float roughness = texture(sampler2D(GMat), a_TexCoord).g;
    float ao =        texture(sampler2D(GMat), a_TexCoord).b;

    vec3 viewPos = u_ViewPos;
    
    vec3 N = normal;
    vec3 V = normalize(viewPos - worldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // reflectance equation
    vec3 Lo = vec3(0.0);

    // Directional Light
    {
        vec3 color = u_DirectionalLight.Color.rgb;
        float intensity = u_DirectionalLight.Params.r;

        // calculate radiance
        vec3 L = normalize(-u_DirectionalLight.Direction.xyz);
        vec3 H = normalize(V + L);
        vec3 radiance = color * intensity;

        Lo = AddLight(N, H, V, L, F0, roughness, metallic, albedo, radiance);
    }

    // Point Lights
    for (int i = 0; i < u_PointLightCount; i++) {
        vec3 position = PointLights[i].Position.xyz;
        vec3 color = PointLights[i].Color.rgb;

        float radius = PointLights[i].Params.r;
        float intensity = PointLights[i].Params.g;

        // calculate per light radiance
        vec3 L = normalize(position - worldPos);
        vec3 H = normalize(V + L);
        float distance = max(length(position - worldPos), 0.0);
        float x = clamp(1.0 - (distance / radius), 0.0, 1.0);
        float attenuation = x * x;
        vec3 radiance = color * attenuation * intensity;

        Lo += AddLight(N, H, V, L, F0, roughness, metallic, albedo, radiance);
    }

    // Spot Lights
    for (int i = 0; i < u_SpotLightCount; i++) {
        vec3 position = SpotLights[i].Position.xyz;
        vec3 direction = SpotLights[i].Direction.xyz;
        vec3 color = SpotLights[i].Color.rgb;

        float cutoff = SpotLights[i].Direction.a;
        float intensity = SpotLights[i].Color.a;

        vec3 L = normalize(position - worldPos);
        vec3 H = normalize(V + L);

        float theta = dot(L, normalize(-direction));

        if (theta > cutoff) {
            vec3 radiance = color * intensity;
            Lo += AddLight(N, H, V, L, F0, roughness, metallic, albedo, radiance);
        }
    }

    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    // vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(u_IrradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(u_PrefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(u_BrdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (diffuse + specular) * ao;

    vec3 color = ambient + Lo;

    if (color == vec3(0.0)) // mainly here for skybox support
        discard;
    
    // HDR tonemapping
    color = TonemapACES(color);
    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));
    
    // Apply fog
    o_FragColor = vec4(color, 1.0);
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

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}