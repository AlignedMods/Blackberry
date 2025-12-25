#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/model/model.hpp"
#include "blackberry/scene/camera.hpp"
#include "blackberry/model/material.hpp"
#include "blackberry/renderer/shader_storage_buffer.hpp"
#include "blackberry/renderer/enviroment_map.hpp"
#include "blackberry/scene/entity.hpp"

namespace Blackberry {

    class Scene; // forward declaration since SceneRenderer will need scene but scene will also need scene renderer

    struct SceneMeshVertex {
        BlVec3 Position;
        BlVec3 Normal;
        BlVec2 TexCoord;
        u32 MaterialIndex = 0;
        u32 ObjectIndex = 0;
        u32 EntityID = 0;
    };

    struct GPUDirectionalLight {
        BlVec4 Direction; // w is unused
        BlVec4 Color; // w is unused
        BlVec4 Params; // g, b, w is unused
    };

    struct alignas(16) GPUPointLight {
        BlVec4 Position; // w is unused
        BlVec4 Color; // w is unused
        BlVec4 Params; // w is unused
    };

    struct alignas(16) GPUSpotLight {
        BlVec4 Position; // w is unused
        BlVec4 Direction; // w is used for cutoff
        BlVec4 Color; // w is used for intensity
    };

    struct alignas(16) GPUMaterial {
        int UseAlbedoTexture = false;
        u64 AlbedoTexture = 0;
        BlVec4 AlbedoColor;

        int UseMetallicTexture = false;
        u64 MetallicTexture = 0;
        f32 MetallicFactor = 0.0f;

        int UseRoughnessTexture = false;
        u64 RoughnessTexture = 0;
        f32 RoughnessFactor = 0.0f;

        int UseAOTexture = false;
        u64 AOTexture = 0;
        f32 AOFactor = 0.0f;

        f32 Emission = 0.0f;
    };

    struct SceneRendererState {
        // shaders
        Ref<Shader> MeshGeometryShader;
        Ref<Shader> MeshLightingShader;
        Ref<Shader> SkyboxShader;

        Ref<Shader> BloomExtractBrightAreasShader;
        Ref<Shader> BloomDownscaleShader;
        Ref<Shader> BloomUpscaleShader;
        Ref<Shader> BloomCombineShader;
        Ref<Shader> ToneMapShader;

        Ref<Shader> FontShader;

        // shader buffers
        ShaderStorageBuffer TransformBuffer;
        ShaderStorageBuffer MaterialBuffer;
        ShaderStorageBuffer ShaderGBuffer;
        ShaderStorageBuffer PointLightBuffer;
        ShaderStorageBuffer SpotLightBuffer;

        std::vector<glm::mat4> Transforms;
        std::vector<GPUMaterial> Materials;
        std::vector<GPUPointLight> PointLights;
        std::vector<GPUSpotLight> SpotLights;
        GPUDirectionalLight DirectionalLight;

        std::vector<SceneMeshVertex> MeshVertices;
        u32 MeshVertexCount = 0;
        std::vector<u32> MeshIndices;
        u32 MeshIndexCount = 0;

        u32 MaterialIndex = 0;
        u32 ObjectIndex = 0;

        // quad vertices (for fullscreen quads or text)
        std::array<f32, 24> QuadVertices = {{
            // pos         // texCoord
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
            -1.0f, -1.0f,  0.0f, 0.0f,   // bottom-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
            
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f    // top-right
        }};
        std::array<u32, 6> QuadIndices = {{ 0, 1, 2, 3, 4, 5 }};

        std::array<f32, 108> CubeVertices = {{
           -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
           -0.5f,  0.5f, -0.5f,
           -0.5f, -0.5f, -0.5f,

           -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
           -0.5f,  0.5f,  0.5f,
           -0.5f, -0.5f,  0.5f,

           -0.5f,  0.5f,  0.5f,
           -0.5f,  0.5f, -0.5f,
           -0.5f, -0.5f, -0.5f,
           -0.5f, -0.5f, -0.5f,
           -0.5f, -0.5f,  0.5f,
           -0.5f,  0.5f,  0.5f,

            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,

           -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
           -0.5f, -0.5f,  0.5f,
           -0.5f, -0.5f, -0.5f,

           -0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
           -0.5f,  0.5f,  0.5f,
           -0.5f,  0.5f, -0.5f,
        }};
        std::array<u32, 36> CubeIndices = {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35 }};

        Ref<Framebuffer> GBuffer; // For deffered rendering

        Ref<Framebuffer> PBROutput; // The rendered image after passing through the PBR shader
        Ref<Framebuffer> BloomBrightAreas; // The areas above the bloom threshold
        std::array<Ref<Framebuffer>, 6> BloomDownscalePasses; // Bloom downscale passes
        std::array<Ref<Framebuffer>, 5> BloomUpscalePasses; // Bloom upscale passes
        Ref<Framebuffer> BloomCombinePass; // Bloom combine pass

        Ref<EnviromentMap> CurrentEnviromentMap;
        Ref<EnviromentMap> DefaultEnviromentMap;
        f32 EnviromentMapLOD = 0.0f;
        BlVec3 EnviromentFogColor;
        f32 EnviromentFogDistance = 0.0f;

        bool BloomEnabled = true;
        f32 BloomThreshold = 3.0f;
    };

    class SceneRenderer {
    public:
        SceneRenderer();

        void Render(Scene* scene);

        void SetCamera(const SceneCamera& camera);
        SceneCamera GetCamera();
        void SetRenderTarget(Ref<Framebuffer> texture);

        // NOTE: this function will not call flush!
        // You must call it yourself afterward
        void RenderEntity(Entity entity);

        void Flush();

        // NOTE: The result from the geometry pass is in m_State.GBuffer
        void GeometryPass();
        // NOTE: The result from the lighting pass in in m_State.PBROutput
        void LightingPass();
        // NOTE: The result from the bloom pass is in m_Target
        void BloomPass();

        // NOTE: The following function MUST be called if you call *.Pass manually!
        void ResetState();

        SceneRendererState& GetState();

    private:
        void AddMesh(const TransformComponent& transform, const Mesh& mesh, const Material& mat, BlColor color, u32 entityID);
        void AddModel(const TransformComponent& transform, const MeshComponent& model, BlColor color, u32 entityID);

        void AddDirectionalLight(const TransformComponent& transform, const DirectionalLightComponent& light);
        void AddPointLight(const TransformComponent& transform, const PointLightComponent& light);
        void AddSpotLight(const TransformComponent& transform, const SpotLightComponent& light);

        void AddEnviroment(const EnviromentComponent& env);

        u32 GetMaterialIndex(const Material& mat);

    private:
        SceneRendererState m_State;

        SceneCamera m_Camera;
        Ref<Framebuffer> m_RenderTarget;
    };

} // namespace Blackberry