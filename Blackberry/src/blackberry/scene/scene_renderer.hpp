#pragma once

#include "blackberry/application/renderer.hpp"

namespace Blackberry {

    class Scene; // forward declaration since SceneRenderer will need scene but scene will also need scene renderer

    struct SceneMeshVertex {
        BlVec3<f32> Position;
        BlVec3<f32> Normal;
        BlVec4<f32> Color;
    };

    struct SceneRendererState {
        std::vector<SceneMeshVertex> MeshVertices;
        std::vector<u32> MeshIndices;
    };

    class SceneRenderer {
    public:
        SceneRenderer(Scene* scene);
        ~SceneRenderer();

        void Render();

    private:
        Scene* m_Scene = nullptr;

    };

} // namespace Blackberry