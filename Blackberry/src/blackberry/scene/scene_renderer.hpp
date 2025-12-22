#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/model/model.hpp"
#include "blackberry/scene/camera.hpp"
#include "blackberry/model/material.hpp"
#include "blackberry/renderer/shader_storage_buffer.hpp"
#include "blackberry/renderer/enviroment_map.hpp"

namespace Blackberry {

    class Scene; // forward declaration since SceneRenderer will need scene but scene will also need scene renderer

    struct SceneMeshVertex {
        BlVec3<f32> Position;
        BlVec3<f32> Normal;
        BlVec2<f32> TexCoord;
        u32 MaterialIndex = 0;
        u32 ObjectIndex = 0;
    };

    struct GPUDirectionalLight {
        BlVec4<f32> Direction; // w is unused
        BlVec4<f32> Color; // w is unused
        BlVec4<f32> Params; // g, b, w is unused
    };

    struct alignas(16) GPUPointLight {
        BlVec4<f32> Position; // w is unused
        BlVec4<f32> Color; // w is unused
        BlVec4<f32> Params; // w is unused
    };

    struct alignas(16) GPUSpotLight {
        BlVec4<f32> Position; // w is unused
        BlVec4<f32> Direction; // w is used for cutoff
        BlVec4<f32> Color; // w is used for intensity
    };

    struct alignas(16) GPUMaterial {
        int UseAlbedoTexture = false;
        u64 AlbedoTexture = 0;
        BlVec4<f32> AlbedoColor;

        int UseMetallicTexture = false;
        u64 MetallicTexture = 0;
        f32 MetallicFactor = 0.0f;

        int UseRoughnessTexture = false;
        u64 RoughnessTexture = 0;
        f32 RoughnessFactor = 0.0f;

        int UseAOTexture = false;
        u64 AOTexture = 0;
        f32 AOFactor = 0.0f;
    };

    struct SceneRendererState {
        // shaders
        Shader MeshGeometryShader;
        Shader MeshLightingShader;
        Shader SkyboxShader;

        Shader BloomExtractBrightAreasShader;
        Shader BloomGaussianBlurShader;
        Shader BloomCombineShader;
        Shader ToneMapShader;

        Shader FontShader;

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

        Ref<RenderTexture> GBuffer; // For deffered rendering

        Ref<RenderTexture> PBROutput; // The rendered image after passing through the PBR shader
        Ref<RenderTexture> BloomBrightAreas; // The areas above the bloom threshold
        Ref<RenderTexture> BloomBlurPass1[2]; // 1st bloom blur pass (two of them because horizontal and vertical pass)
        Ref<RenderTexture> BloomBlurPass2[2]; // 2nd boom blur pass
        Ref<RenderTexture> BloomBlurPass3[2]; // you get the god dam point
        Ref<RenderTexture> BloomBlurPass4[2];
        Ref<RenderTexture> BloomBlurPass5[2];

        Ref<RenderTexture> BloomCombinePass1;
        Ref<RenderTexture> BloomCombinePass2;
        Ref<RenderTexture> BloomCombinePass3;
        Ref<RenderTexture> BloomCombinePass4;
        Ref<RenderTexture> BloomCombinePass5;
        Ref<RenderTexture> BloomCombinePass6;

        f32 BloomThreshold = 3.0f;

        Ref<EnviromentMap> CurrentEnviromentMap;
        Ref<EnviromentMap> DefaultEnviromentMap;
        f32 EnviromentMapLOD = 0.0f;
        BlVec3<f32> EnviromentFogColor;
        f32 EnviromentFogDistance = 0.0f;
    };

    class SceneRenderer {
    public:
        SceneRenderer();

        void Render(Scene* scene, RenderTexture* target);
        void SetCamera(const SceneCamera& camera);

        SceneRendererState& GetState();

    private:
        void AddMesh(const TransformComponent& transform, const Mesh& mesh, const Material& mat, BlColor color);
        void AddModel(const TransformComponent& transform, const MeshComponent& model, BlColor color);

        void AddDirectionalLight(const TransformComponent& transform, const DirectionalLightComponent& light);
        void AddPointLight(const TransformComponent& transform, const PointLightComponent& light);
        void AddSpotLight(const TransformComponent& transform, const SpotLightComponent& light);

        void AddEnviroment(const EnviromentComponent& env);

        void Flush();

        void BloomPipeline();

        u32 GetMaterialIndex(const Material& mat);

    private:
        SceneRendererState m_State;

        SceneCamera m_Camera;
        RenderTexture* m_Target = nullptr;
    };

} // namespace Blackberry