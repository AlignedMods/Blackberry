#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/model/model.hpp"
#include "blackberry/scene/camera.hpp"
#include "blackberry/model/material.hpp"

namespace Blackberry {

    class Scene; // forward declaration since SceneRenderer will need scene but scene will also need scene renderer

    struct SceneMeshVertex {
        BlVec3<f32> Position;
        BlVec3<f32> Normal;
        BlVec2<f32> TexCoord;
        u32 MaterialIndex = 0;
    };

    struct SceneDirectionalLight {
        BlVec3<f32> Direction;

        BlVec3<f32> Ambient;
        BlVec3<f32> Diffuse;
        BlVec3<f32> Specular;
    };

    struct SceneRendererState {
        // shaders
        Shader MeshShader;
        Shader FontShader;

        SceneDirectionalLight DirectionalLight;

        std::vector<SceneMeshVertex> MeshVertices;
        u32 MeshVertexCount = 0;
        std::vector<u32> MeshIndices;
        u32 MeshIndexCount = 0;

        std::array<Material, 16> Materials;
        u32 MaterialIndex = 0; // 0 is reserved for meshes without a material
    };

    class SceneRenderer {
    public:
        SceneRenderer();

        void Render(Scene* scene);
        void SetCamera(const SceneCamera& camera);

    private:
        void AddMesh(const glm::mat4& transform, const Mesh& mesh, BlColor color);
        void AddModel(const glm::mat4& transform, const Model& model, BlColor color);

        void AddDirectionalLight(const TransformComponent& transform, const DirectionalLightComponent& light);

        void Flush();

        u32 GetMaterialIndex(const Material& mat);

    private:
        SceneRendererState m_State;

        SceneCamera m_Camera;
    };

} // namespace Blackberry