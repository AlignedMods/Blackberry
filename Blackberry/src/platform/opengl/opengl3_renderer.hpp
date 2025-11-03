#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/rendering/shader.hpp"

#include "glm/glm.hpp"

#include <vector>

namespace Blackberry {

    class Renderer_OpenGL3 : public Renderer {
    public:
        explicit Renderer_OpenGL3(BlVec2 viewport);

        virtual void UpdateViewport(BlVec2 viewport) override;

        virtual void NewFrame() override;
        virtual void EndFrame() override;

        virtual void Clear(BlColor color) const override;

        virtual BlVec2 GetViewportSize() const override;

        virtual void SetBufferLayout(const BlDrawBufferLayout& layout) override;

        virtual void SubmitDrawBuffer(const BlDrawBuffer& buffer) override;
        virtual void DrawIndexed(u32 count) override;
        
        virtual void BindShader(BlShader shader) override;

        virtual void AttachTexture(BlTexture texture, u32 slot = 0) override;
        virtual void DetachTexture() override;

        virtual void AttachRenderTexture(const BlRenderTexture texture) override;
        virtual void DetachRenderTexture() override;

        virtual void SetProjection(glm::mat4 projection) override;
        virtual void ResetProjection() override;

    private:
        u32 m_VAO;
        u32 m_VBO;
        u32 m_EBO;
    
        // shaders
        BlShader m_ShapeShader;
        BlShader m_TextureShader;

        BlShader m_CurrentShader;

        BlVec2 m_CurrentViewportSize;
        BlVec2 m_PrevViewportSize;

        glm::mat4 m_Projection;
        glm::mat4 m_DefaultProjection;
    };

} // namespace Blackberry
