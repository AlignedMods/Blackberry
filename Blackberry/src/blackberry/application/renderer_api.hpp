#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/renderer/shader.hpp"
#include "blackberry/renderer/vertex_buffer.hpp"

namespace Blackberry {

    enum class RendererCapability {
        Blend,
        FaceCull,
        DepthTest,
        ScissorTest,
        StencilTest,
        SeamlessCubemap
    };

    enum class BlendFunc {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha
    };

    enum class BlendEquation {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    enum class DepthFunc {
        Never,
        Less,
        Equal,
        Lequal,
        Greater,
        NotEqual,
        Gequal,
        Always
    };

    class RendererAPI {
    public:
        virtual void SetViewportSize(BlVec2 size) const = 0;
        virtual void ClearFramebuffer(const BlVec4& color = BlVec4(0.0f)) const = 0;

        virtual void EnableCapability(RendererCapability cap) const = 0;
        virtual void DisableCapability(RendererCapability cap) const = 0;

        virtual void SetBlendFunc(BlendFunc func1, BlendFunc func2) const = 0;
        virtual void SetBlendEquation(BlendEquation eq) const = 0;

        virtual void SetDepthFunc(DepthFunc func) const = 0;
        virtual void SetDepthMask(bool mask) const = 0;
        
        virtual void DrawVertexArray(const Ref<VertexArray>& vertexArray) const = 0;

        virtual void BindShader(const Ref<Shader>& shader) const = 0;

        virtual void BindFramebuffer(const Ref<Framebuffer>& framebuffer) const = 0;
        virtual void UnBindFramebuffer() const = 0;

        virtual void BindTexture2D(const Ref<Texture2D>& texture, u32 slot = 0) const = 0;
        virtual void UnBindTexture2D() const = 0;

        virtual void BindTextureCubemap(const Ref<Texture2D>& texture, u32 slot = 0) const = 0;
        virtual void UnBindTextureCubemap() const = 0;
    };

} // namespace Blackberry
