#include "blackberry/renderer/debug_renderer.hpp"
#include "blackberry/renderer/shader.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/scene/scene_renderer.hpp"

namespace Blackberry {

    struct DebugRendererState {
        Ref<Framebuffer> DummyRenderTexture; // Dummy framebuffer used to not render certian things

        Ref<Shader> MaskShader;
        Ref<Shader> OutlineShader;

        Ref<Framebuffer> TargetTexture;

        Ref<VertexArray> QuadVAO;
        Ref<VertexArray> CubeVAO;
    };

    static DebugRendererState s_DebugRendererState;

    void DebugRenderer::Initialize() {
        FramebufferSpecification spec;
        spec.Width = 1920;
        spec.Height = 1080;
        spec.Attachments = {
            {0, FramebufferAttachmentType::ColorRGBA8},
            {1, FramebufferAttachmentType::Depth}
        };
        spec.ActiveAttachments = {0};

        s_DebugRendererState.DummyRenderTexture = Framebuffer::Create(spec);

        s_DebugRendererState.OutlineShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Debug/Outline.frag"));
        s_DebugRendererState.MaskShader = Shader::Create(FS::Path("Assets/Shaders/Default/Core/Quad.vert"), FS::Path("Assets/Shaders/Default/Core/FlatColor.frag"));

        // quad vertices (for fullscreen quads or text)
        std::array<f32, 24> QuadVertices = {{
            // pos         // texCoord
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
            -1.0f, -1.0f,  0.0f, 0.0f,   // bottom-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
            
            -1.0f,  1.0f,  0.0f, 1.0f,   // top-left
             1.0f, -1.0f,  1.0f, 0.0f,   // bottom-right
             1.0f,  1.0f,  1.0f, 1.0f    // top-right
        }};

        Ref<VertexBuffer> quadVertexBuffer = VertexBuffer::Create(QuadVertices.data(), sizeof(f32) * 4, 6, BufferUsage::Static);
        s_DebugRendererState.QuadVAO = VertexArray::Create();
        s_DebugRendererState.QuadVAO->SetVertexBuffer(quadVertexBuffer);
        s_DebugRendererState.QuadVAO->SetVertexLayout({
            {0, ShaderDataType::Float2, "Position"},
            {1, ShaderDataType::Float2, "TexCoord"}
        });

        std::array<f32, 288> CubeVertices = {{
           // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        }};

        Ref<VertexBuffer> cubeVertexBuffer = VertexBuffer::Create(CubeVertices.data(), sizeof(f32) * 8, 36, BufferUsage::Static);
        s_DebugRendererState.CubeVAO = VertexArray::Create();
        s_DebugRendererState.CubeVAO->SetVertexBuffer(cubeVertexBuffer);
        s_DebugRendererState.CubeVAO->SetVertexLayout({
            {0, ShaderDataType::Float3, "Position"},
            {1, ShaderDataType::Float2, "TexCoord"},
            {2, ShaderDataType::Float3, "Normal"},
        });
    }

    void DebugRenderer::Shutdown() {}

    void DebugRenderer::SetRenderTarget(Ref<Framebuffer> target) {
        auto& api = BL_APP.GetRendererAPI();

        s_DebugRendererState.TargetTexture = target;

        api.BindFramebuffer(target);
        api.ClearFramebuffer();

        api.UnBindFramebuffer();
    }

    void DebugRenderer::DrawEntityOutline(Entity e) {
        if (!e.HasComponent<TransformComponent>() || !e.HasComponent<MeshComponent>()) return;

        auto& sceneRenderer = *e.EntityScene->GetSceneRenderer();
        auto& api = BL_APP.GetRendererAPI();

        sceneRenderer.RenderEntity(e);
        sceneRenderer.GeometryPass(); // Only do the geometry pass since doing the lighting pass would do PBR calculations

        sceneRenderer.ResetState(); // also reset state since Flush normally does this but we aren't calling flush

        Ref<Texture> entities = sceneRenderer.GetState().GBuffer->Attachments[4];

        api.BindShader(s_DebugRendererState.MaskShader);

        api.BindFramebuffer(s_DebugRendererState.DummyRenderTexture);
        api.ClearFramebuffer();

        api.BindTexture2D(entities, 0);
        api.DrawVertexArray(s_DebugRendererState.QuadVAO);
        api.UnBindTexture2D();

        api.BindShader(s_DebugRendererState.OutlineShader);

        s_DebugRendererState.OutlineShader->SetVec2("u_TexelSize", BlVec2(1.0f / s_DebugRendererState.TargetTexture->Specification.Width, 1.0f / s_DebugRendererState.TargetTexture->Specification.Height));
        s_DebugRendererState.OutlineShader->SetFloat("u_Thickness", 4.0f);
        s_DebugRendererState.OutlineShader->SetVec3("u_OutlineColor", BlVec3(1.0f, 0.5f, 0.1f));

        api.BindFramebuffer(s_DebugRendererState.TargetTexture);

        api.BindTexture2D(s_DebugRendererState.DummyRenderTexture->Attachments[0], 0);
        api.DrawVertexArray(s_DebugRendererState.QuadVAO);
        api.UnBindTexture2D();

        api.UnBindFramebuffer();
    }

    Ref<VertexArray>& DebugRenderer::GetQuadVAO() {
        return s_DebugRendererState.QuadVAO;
    }

    Ref<VertexArray>& DebugRenderer::GetCubeVAO() {
        return s_DebugRendererState.CubeVAO;
    }

} // namespace Blackberry