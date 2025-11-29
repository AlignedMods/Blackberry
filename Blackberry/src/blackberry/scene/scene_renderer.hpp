#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/model/model.hpp"
#include "blackberry/scene/camera.hpp"

namespace Blackberry {

    class Scene; // forward declaration since SceneRenderer will need scene but scene will also need scene renderer

    struct SceneMeshVertex {
        BlVec3<f32> Position;
        BlVec3<f32> Normal;
        BlVec4<f32> Color;
    };

    struct SceneRendererState {
        // shaders
        Shader MeshShader;
        Shader FontShader;

        DirectionalLightComponent DirectionalLight;

        std::vector<SceneMeshVertex> MeshVertices;
        u32 MeshVertexCount = 0;
        std::vector<u32> MeshIndices;
        u32 MeshIndexCount = 0;
    };

    class SceneRenderer {
    public:
        SceneRenderer();

        void Render(Scene* scene);
        void SetCamera(const SceneCamera& camera);

    private:
        void AddMesh(const glm::mat4& transform, const Mesh& mesh, BlColor color);
        void AddModel(const glm::mat4& transform, const Model& model, BlColor color);

        void AddDirectionalLight(const DirectionalLightComponent& light);

        void Flush();

    private:
        SceneRendererState m_State;

        SceneCamera m_Camera;
    };

} // namespace Blackberry