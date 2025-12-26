#pragma once

#include "blackberry/renderer/texture.hpp"
#include "blackberry/scene/entity.hpp"

namespace Blackberry {

    class DebugRenderer {
    public:
        static void Initialize();
        static void Shutdown();

        static void SetRenderTarget(Ref<Framebuffer> target);

        static void DrawEntityOutline(Entity e);

        static Ref<VertexArray>& GetQuadVAO();
        static Ref<VertexArray>& GetCubeVAO();
    };

} // namespace Blackberry