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

        virtual void Begin(RenderingMode mode) override;
        virtual void End() override;

        virtual void SubVertex(const BlVertex& vert) override;

        virtual void AttachDefaultShader(DefaultShader shader) override;

        virtual void Clear() const override;

        virtual void AttachTexture(const BlTexture tex) override;
        virtual void DettachTexture() override;

        virtual void AttachRenderTexture(const BlRenderTexture texture) override;
        virtual void DettachRenderTexture() override;

        virtual u32 GetDrawCalls() const override { return m_DrawCalls; }

    private:
        virtual void Render() override;

        void CompileDefaultShaders();

    private:
        u32 m_VAO;
        u32 m_VBO;
    
        // shaders
        u32 m_ShapeShader;
        u32 m_TextureShader;
        u32 m_FontShader;

        u32 m_CurrentShader;

        f32 m_Vertices[2000];
        u64 m_VertexIndex = 0;
        u64 m_CurrentVertex = 0;

        glm::mat4 m_Projection;

        RenderingMode m_Mode;

        bool m_UsingTexture = false;
        BlTexture m_CurrentTexture;

        u32 m_DrawCalls = 0;
    };

} // namespace Blackberry
