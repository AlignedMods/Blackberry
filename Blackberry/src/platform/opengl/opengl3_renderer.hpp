#pragma once

#include "blackberry/application/renderer.hpp"
#include "blackberry/core/types.hpp"
#include "blackberry/renderer/shader.hpp"

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

        virtual void SetBufferLayout(const std::initializer_list<DrawBufferLayout>& layout) override;

        virtual void SubmitDrawBuffer(const DrawBuffer& buffer) override;
        virtual void DrawIndexed(u32 count) override;
        
        virtual void BindShader(Shader shader) override;

        virtual void BindTexture(Ref<Texture2D> texture, u32 unit) override;
        virtual void UnBindTexture() override;

        virtual void BindCubemap(Ref<Texture2D> cubemap, u32 unit) override;
        virtual void UnBindCubemap() override;

        virtual void BindRenderTexture(Ref<RenderTexture> texture) override;
        virtual void UnBindRenderTexture() override;

    private:
        u32 m_VAO;
        u32 m_VBO;
        u32 m_EBO;
    
        // shaders
        Shader m_ShapeShader;
        Shader m_TextureShader;

        Shader m_CurrentShader;

        BlVec2 m_CurrentViewportSize;
        BlVec2 m_PrevViewportSize;

        glm::mat4 m_Projection;
        glm::mat4 m_DefaultProjection;
    };

} // namespace Blackberry
