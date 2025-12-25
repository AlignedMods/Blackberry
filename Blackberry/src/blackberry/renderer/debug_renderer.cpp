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
    }

    void DebugRenderer::Shutdown() {}

    void DebugRenderer::SetRenderTarget(Ref<Framebuffer> target) {
        auto& renderer = BL_APP.GetRenderer();

        s_DebugRendererState.TargetTexture = target;

        renderer.BindFramebuffer(target);
        renderer.Clear(BlColor(0, 0, 0, 255));

        renderer.UnBindFramebuffer();
    }

    void DebugRenderer::DrawEntityOutline(Entity e) {
        if (!e.HasComponent<TransformComponent>() || !e.HasComponent<MeshComponent>()) return;

        auto& sceneRenderer = *e.EntityScene->GetSceneRenderer();
        auto& renderer = BL_APP.GetRenderer();

        sceneRenderer.RenderEntity(e);
        sceneRenderer.GeometryPass(); // Only do the geometry pass since doing the lighting pass would do PBR calculations

        sceneRenderer.ResetState(); // also reset state since Flush normally does this but we aren't calling flush

        Ref<Texture2D> entities = sceneRenderer.GetState().GBuffer->Attachments[4];

        DrawBuffer quad;
        quad.Vertices = sceneRenderer.GetState().QuadVertices.data();
        quad.VertexCount = 6;
        quad.VertexSize = sizeof(f32) * 4;

        quad.Indices = sceneRenderer.GetState().QuadIndices.data();
        quad.IndexCount = 6;
        quad.IndexSize = sizeof(u32);

        renderer.SubmitDrawBuffer(quad);
        renderer.SetBufferLayout({
            {0, ShaderDataType::Float2, "Position"},
            {1, ShaderDataType::Float2, "TexCoord"},
        });

        renderer.BindShader(s_DebugRendererState.MaskShader);

        renderer.BindFramebuffer(s_DebugRendererState.DummyRenderTexture);
        renderer.Clear(BlColor(0, 0, 0, 255));

        renderer.BindTexture(entities, 0);
        renderer.DrawIndexed(6);
        renderer.UnBindTexture();

        renderer.BindShader(s_DebugRendererState.OutlineShader);

        s_DebugRendererState.OutlineShader->SetVec2("u_TexelSize", BlVec2(1.0f / s_DebugRendererState.TargetTexture->Specification.Width, 1.0f / s_DebugRendererState.TargetTexture->Specification.Height));
        s_DebugRendererState.OutlineShader->SetFloat("u_Thickness", 4.0f);
        s_DebugRendererState.OutlineShader->SetVec3("u_OutlineColor", BlVec3(1.0f, 0.5f, 0.1f));

        renderer.BindFramebuffer(s_DebugRendererState.TargetTexture);

        renderer.BindTexture(s_DebugRendererState.DummyRenderTexture->Attachments[0], 0);
        renderer.DrawIndexed(6);
        renderer.UnBindTexture();

        renderer.UnBindFramebuffer();
    }

} // namespace Blackberry