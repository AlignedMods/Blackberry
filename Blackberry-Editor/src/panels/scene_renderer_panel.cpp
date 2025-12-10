#include "scene_renderer_panel.hpp"

#include "blackberry.hpp"

using namespace Blackberry;

namespace BlackberryEditor {
    
    void SceneRendererPanel::OnUIRender(bool& open) {
        if (!open) return;

        ImGui::Begin("Scene Renderer View", &open);
        
        auto* renderer = m_Context->GetSceneRenderer();
        auto& state = renderer->GetState();

        static const char* names[] = { "Positions", "Normals", "Albedo", "Material" };
        const char* name = names[m_CurrentDeferredImage];

        f32 sizeX = ImGui::GetContentRegionAvail().x;
        f32 sizeY = sizeX / 1.7778f;

        ImGui::SliderInt("Deferred rendering step", &m_CurrentDeferredImage, 0, IM_ARRAYSIZE(names) - 1, name);
        ImGui::Image(state.GBuffer->Attachments[m_CurrentDeferredImage]->ID, ImVec2(sizeX, sizeY), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
    }

    void SceneRendererPanel::SetContext(Scene* scene) {
        m_Context = scene;
    }

} // namespace BlackberryEditor