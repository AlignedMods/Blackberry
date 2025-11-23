#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/image/image.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/renderer/shader.hpp"

struct BlDrawBuffer {
    std::string DebugName;

    void* Vertices; // vertex data could be any struct, so we use void*
    u32 VertexCount = 0;
    u32 VertexSize = 0; // size of a single vertex (bytes)

    u32* Indices = nullptr; // index data (u32 array)
    u32 IndexCount = 0;
    u32 IndexSize = 0; // size of a single index (bytes)
};

struct BlDrawBufferLayout {
    u32 Index = 0; // binding index
    u32 Count = 0; // amount of elements
    enum class ElementType {
        Float,
        Float2,
        Float3,
        Float4,
        UByte4,
        UShort,
        UInt
    } Type; // type of each element
    u32 Stride = 0; // space between the current element and the next one (bytes)
    u32 Offset = 0; // offset from the start of the vertex (bytes)
};

namespace Blackberry {

    class Renderer {
    public:
        virtual ~Renderer() = default;

        virtual void UpdateViewport(BlVec2<f32> viewport) = 0;

        virtual void NewFrame() = 0;
        virtual void EndFrame() = 0;

        virtual void Clear(BlColor color) const = 0;

        virtual BlVec2<f32> GetViewportSize() const = 0;

        virtual void SetBufferLayout(const BlDrawBufferLayout& layout) = 0;

        virtual void SubmitDrawBuffer(const BlDrawBuffer& buffer) = 0;
        virtual void DrawIndexed(u32 count) = 0;

        virtual void BindShader(BlShader shader) = 0;

        virtual void BindTexture(Texture2D texture, u32 slot = 0) = 0;
        virtual void UnBindTexture() = 0;

        virtual void BindRenderTexture(const RenderTexture texture) = 0;
        virtual void UnBindRenderTexture() = 0;
    };

} // namespace Blackberry
