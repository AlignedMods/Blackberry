#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/renderer/shader.hpp"

#include "glm/glm.hpp"

#include <vector>

namespace Blackberry {

    class Renderer_OpenGL3 : public Renderer {
    public:
        explicit Renderer_OpenGL3(BlVec2<f32> viewport);

        virtual void UpdateViewport(BlVec2<f32> viewport) override;

        virtual void NewFrame() override;
        virtual void EndFrame() override;

        virtual void Clear(BlColor color) const override;

        virtual BlVec2<f32> GetViewportSize() const override;

        virtual void SetBufferLayout(const std::initializer_list<DrawBufferLayout>& layout) override;

        virtual void SubmitDrawBuffer(const DrawBuffer& buffer) override;
        virtual void DrawIndexed(u32 count) override;
        
        virtual void BindShader(Shader shader) override;

        virtual void BindRenderTexture(const RenderTexture texture) override;
        virtual void UnBindRenderTexture() override;

    private:
        u32 m_VAO;
        u32 m_VBO;
        u32 m_EBO;
    
        // shaders
        Shader m_ShapeShader;
        Shader m_TextureShader;

        Shader m_CurrentShader;

        BlVec2<f32> m_CurrentViewportSize;
        BlVec2<f32> m_PrevViewportSize;

        glm::mat4 m_Projection;
        glm::mat4 m_DefaultProjection;
    };

} // namespace Blackberry
