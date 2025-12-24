#pragma once

#include "blackberry/renderer/texture.hpp"
#include "blackberry/scene/entity.hpp"

namespace Blackberry {

    class DebugRenderer {
    public:
        static void Initialize();
        static void Shutdown();

        static void SetRenderTexture(Ref<RenderTexture> render);
        static void ResetRenderTexture();

        static void DrawEntityOutline(Entity e);
    };

} // namespace Blackberry