#pragma once

#include "blackberry/types.hpp"
#include "blackberry/image/image.hpp"
#include "blackberry/rendering/texture.hpp"

enum class RenderingMode {
    Points,
    Lines,
    Triangles
};

struct BlVertex {
    inline explicit BlVertex(BlVec2 pos, BlColor color, BlVec2 texCoord)
        : pos(pos), color(color), texCoord(texCoord) {}

    BlVec2 pos;
    BlColor color;
    BlVec2 texCoord;
};

namespace Blackberry {

    class Renderer {
    public:
        virtual ~Renderer() = default;

        virtual void UpdateViewport(BlVec2 viewport) = 0;

        virtual void NewFrame() = 0;

        virtual void Begin(RenderingMode mode) = 0;
        virtual void End() = 0;

        virtual void Clear() const = 0;

        virtual void SubVertex(const BlVertex& vert) = 0;

        virtual void AttachTexture(const BlTexture texture) = 0;
        virtual void DettachTexture() = 0;

        virtual void AttachRenderTexture(const BlRenderTexture texture) = 0;
        virtual void DettachRenderTexture() = 0;

        virtual u32 GetDrawCalls() const = 0;

    protected:
        virtual void Render() = 0;
    };

} // namespace Blackberry
