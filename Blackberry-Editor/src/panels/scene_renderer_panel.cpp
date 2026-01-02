#include "scene_renderer_panel.hpp"

#include "blackberry.hpp"

using namespace Blackberry;

namespace BlackberryEditor {
    
    void SceneRendererPanel::OnUIRender(bool& open) {
        if (!open) return;

        ImGui::Begin("Scene Renderer View", &open);

        ImGui::Text("FPS: %f", 1.0f / BL_APP.GetDeltaTime());
        ImGui::Text("Rendering time: %fms", BL_APP.GetDeltaTime() * 1000.0f);

        ImGui::Text("SceneRenderer::Render: %fms", Instrumentor::GetTimePoint("SceneRenderer::Render").Milliseconds());
        ImGui::Text("SceneRenderer::Flush: %fms", Instrumentor::GetTimePoint("SceneRenderer::Flush").Milliseconds());

        ImGui::Separator();

        ImGui::Text("SceneRenderer::GeometryPass %fms", Instrumentor::GetTimePoint("SceneRenderer::GeometryPass").Milliseconds());
        ImGui::Text("SceneRenderer::LightingPass %fms", Instrumentor::GetTimePoint("SceneRenderer::LightingPass").Milliseconds());
        ImGui::Text("SceneRenderer::BloomPass %fms", Instrumentor::GetTimePoint("SceneRenderer::BloomPass").Milliseconds());
        // ImGui::Text("Draw calls: %d", BL_APP.GetRendererAPI().GetDrawCallCount());

        auto* renderer = m_Context->GetSceneRenderer();
        auto& state = renderer->GetState();

        f32 sizeX = ImGui::GetContentRegionAvail().x;
        f32 sizeY = sizeX / 1.7778f;
        
        if (ImGui::CollapsingHeader("Deffered Renderer")) {
            static const char* names[] = { "Positions", "Normals", "Albedo", "Material" };
            const char* name = names[m_CurrentDeferredImage];

            ImGui::SliderInt("Deferred rendering step", &m_CurrentDeferredImage, 0, IM_ARRAYSIZE(names) - 1, name);
            ImGui::Image(state.GBuffer->Attachments[m_CurrentDeferredImage]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));
        }

        if (ImGui::CollapsingHeader("Bloom Stage")) {
            ImGui::SliderInt("Current Down sampling Stage", &m_CurrentBloomDownsampleStage, 1, 8);

            ImGui::Image(state.BloomDownscalePasses[m_CurrentBloomDownsampleStage - 1]->Attachments[0]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));

            ImGui::SliderInt("Current Up sampling Stage", &m_CurrentBloomUpsamlingStage, 1, 7);

            ImGui::Image(state.BloomUpscalePasses[m_CurrentBloomUpsamlingStage - 1]->Attachments[0]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::Image(state.BloomCombinePass->Attachments[0]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
    }

    void SceneRendererPanel::SetContext(Ref<Scene> scene) {
        m_Context = scene;
    }

} // namespace BlackberryEditor