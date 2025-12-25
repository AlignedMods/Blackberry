#include "scene_renderer_panel.hpp"

#include "blackberry.hpp"

using namespace Blackberry;

namespace BlackberryEditor {
    
    void SceneRendererPanel::OnUIRender(bool& open) {
        if (!open) return;

        ImGui::Begin("Scene Renderer View", &open);

        ImGui::Text("FPS: %f", 1.0f / BL_APP.GetDeltaTime());
        ImGui::Text("Rendering time: %fms", BL_APP.GetDeltaTime() * 1000.0f);
        
        auto* renderer = m_Context->GetSceneRenderer();
        auto& state = renderer->GetState();

        static const char* names[] = { "Positions", "Normals", "Albedo", "Material" };
        const char* name = names[m_CurrentDeferredImage];

        f32 sizeX = ImGui::GetContentRegionAvail().x;
        f32 sizeY = sizeX / 1.7778f;

        ImGui::SliderInt("Deferred rendering step", &m_CurrentDeferredImage, 0, IM_ARRAYSIZE(names) - 1, name);
        ImGui::Image(state.GBuffer->Attachments[m_CurrentDeferredImage]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));

        // ImGui::Image(state.CurrentEnviromentMap->BrdfLUT->ID, ImVec2(sizeX, sizeY));

        ImGui::DragFloat("Bloom Threshold", &state.BloomThreshold, 0.1f);

        ImGui::Text("SceneRenderer::Render: %fms", Instrumentor::GetTimePoint("SceneRenderer::Render").Milliseconds());
        ImGui::Text("SceneRenderer::Flush: %fms", Instrumentor::GetTimePoint("SceneRenderer::Flush").Milliseconds());

        if (ImGui::CollapsingHeader("Bloom Stage")) {
            ImGui::SliderInt("Current Down sampling Stage", &m_CurrentBloomDownsampleStage, 1, 5);

            Ref<Framebuffer> dtex;

            switch (m_CurrentBloomDownsampleStage) {
                case 1: dtex = state.BloomBlurPass1[1]; break;
                case 2: dtex = state.BloomBlurPass2[1]; break;
                case 3: dtex = state.BloomBlurPass3[1]; break;
                case 4: dtex = state.BloomBlurPass4[1]; break;
                case 5: dtex = state.BloomBlurPass5[1]; break;
                default: dtex = state.BloomBlurPass1[1]; break;
            }

            ImGui::Image(dtex->Attachments[0]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));

            ImGui::SliderInt("Current Up sampling Stage", &m_CurrentBloomUpsamlingStage, 1, 5);

            Ref<Framebuffer> utex;

            switch (m_CurrentBloomUpsamlingStage) {
                case 1: utex = state.BloomCombinePass1; break;
                case 2: utex = state.BloomCombinePass2; break;
                case 3: utex = state.BloomCombinePass3; break;
                case 4: utex = state.BloomCombinePass4; break;
                case 5: utex = state.BloomCombinePass5; break;
                default: utex = state.BloomCombinePass1; break;
            }

            ImGui::Image(utex->Attachments[0]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
    }

    void SceneRendererPanel::SetContext(Ref<Scene> scene) {
        m_Context = scene;
    }

} // namespace BlackberryEditor