#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/core/types.hpp"

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

        virtual void SetBufferLayout(const BlDrawBufferLayout& layout) override;

        virtual void SubmitDrawBuffer(const BlDrawBuffer& buffer) override;
        virtual void DrawIndexed(u32 count) override;
        
        virtual void AttachDefaultShader(DefaultShader shader) override;

        virtual void AttachRenderTexture(const BlRenderTexture texture) override;
        virtual void DetachRenderTexture() override;

    private:
        void CompileDefaultShaders();

    private:
        u32 m_VAO;
        u32 m_VBO;
        u32 m_EBO;
    
        // shaders
        u32 m_ShapeShader;
        u32 m_TextureShader;
        u32 m_FontShader;

        u32 m_CurrentShader;

        BlVec2 m_CurrentViewportSize;
        BlVec2 m_PrevViewportSize;

        glm::mat4 m_Projection;
    };

} // namespace Blackberry
