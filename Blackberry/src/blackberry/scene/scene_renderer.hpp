#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/model/model.hpp"
#include "blackberry/scene/camera.hpp"
#include "blackberry/model/material.hpp"
#include "blackberry/renderer/shader_storage_buffer.hpp"

namespace Blackberry {

    class Scene; // forward declaration since SceneRenderer will need scene but scene will also need scene renderer

    struct SceneMeshVertex {
        BlVec3<f32> Position;
        BlVec3<f32> Normal;
        BlVec2<f32> TexCoord;
        u32 MaterialIndex = 0;
        u32 ObjectIndex = 0;
    };

    struct SceneDirectionalLight {
        BlVec3<f32> Direction;

        BlVec3<f32> Ambient;
        BlVec3<f32> Diffuse;
        BlVec3<f32> Specular;
    };

    // NOTE: we need to pad the vec3s since opengl expects vec3s to be 16 bytes
    struct alignas(16) SceneLight {
        BlVec4<f32> Position; // w is unused
        BlVec4<f32> Color; // w is unused
        BlVec4<f32> Params; // w is unused
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
        Shader FontShader;

        // shader buffers
        ShaderStorageBuffer TransformBuffer;
        ShaderStorageBuffer MaterialBuffer;
        ShaderStorageBuffer ShaderGBuffer;
        ShaderStorageBuffer LightBuffer;

        std::vector<glm::mat4> Transforms;
        std::vector<SceneLight> Lights;

        std::vector<SceneMeshVertex> MeshVertices;
        u32 MeshVertexCount = 0;
        std::vector<u32> MeshIndices;
        u32 MeshIndexCount = 0;

        std::vector<GPUMaterial> Materials;
        u32 MaterialIndex = 0;
        u32 ObjectIndex = 0;

        // quad vertices (for fullscreen quads or text)
        std::array<f32, 24> QuadVertices = {{
            // pos      // texCoord
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
            -1.0f, -1.0f,  0.0f, 0.0f,   // bottom-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
            
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f    // top-right
        }};
        std::array<u32, 6> QuadIndices = {{ 0, 1, 2, 3, 4, 5 }};

        Ref<RenderTexture> GBuffer; // For deffered rendering
    };

    class SceneRenderer {
    public:
        SceneRenderer();

        void Render(Scene* scene, RenderTexture* target);
        void SetCamera(const SceneCamera& camera);

        SceneRendererState& GetState();

    private:
        void AddMesh(const glm::mat4& transform, const Mesh& mesh, BlColor color);
        void AddModel(const glm::mat4& transform, const Model& model, BlColor color);

        void AddDirectionalLight(const TransformComponent& transform, const DirectionalLightComponent& light);
        void AddPointLight(const TransformComponent& transform, const PointLightComponent& light);

        void Flush();

        u32 GetMaterialIndex(const Material& mat);

    private:
        SceneRendererState m_State;

        SceneCamera m_Camera;
        RenderTexture* m_Target = nullptr;
    };

} // namespace Blackberry