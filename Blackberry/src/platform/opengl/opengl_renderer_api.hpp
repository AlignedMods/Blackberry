#pragma once

#include "blackberry/application/renderer_api.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/renderer/shader.hpp"

#include "glm/glm.hpp"

#include <vector>

namespace Blackberry {

    class OpenGLRendererAPI : public RendererAPI {
    public:
        OpenGLRendererAPI();

        virtual void SetViewportSize(BlVec2 size) const override;
        virtual void ClearFramebuffer(const BlVec4& color = BlVec4(0.0f)) const override;

        virtual void EnableCapability(RendererCapability cap) const override;
        virtual void DisableCapability(RendererCapability cap) const override;

        virtual void SetBlendFunc(BlendFunc func1, BlendFunc func2) const override;
        virtual void SetBlendEquation(BlendEquation eq) const override;

        virtual void SetDepthFunc(DepthFunc func) const override;
        virtual void SetDepthMask(bool mask) const override;
        
        virtual void DrawVertexArray(const Ref<VertexArray>& vertexArray) const override;

        virtual void BindShader(const Ref<Shader>& shader) const override;

        virtual void BindFramebuffer(const Ref<Framebuffer>& framebuffer) const override;
        virtual void UnBindFramebuffer() const override;

        virtual void BindTexture2D(const Ref<Texture>& texture, u32 slot = 0) const override;
        virtual void UnBindTexture2D() const override;

        virtual void BindTextureCubemap(const Ref<Texture>& texture, u32 slot = 0) const override;
        virtual void UnBindTextureCubemap() const override;

    private:
        mutable BlVec2 m_CurrentFramebufferSize;
        mutable BlVec2 m_PreviousFramebufferSize;
    };

} // namespace Blackberry
