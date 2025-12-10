#include "material_editor_panel.hpp"

using namespace Blackberry;

namespace BlackberryEditor {

    void MaterialEditorPanel::OnUIRender(bool& open) {
        if (!open) return;

        if (ImGui::Begin("Material Editor", &open)) {
            if (Project::GetAssetManager().ContainsAsset(m_Context)) {
                Material& mat = std::get<Material>(Project::GetAssetManager().GetAsset(m_Context).Data);

                ImGui::Image(mat.Albedo->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                ImGui::Text("Albedo");

                ImGui::Image(mat.Metallic->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                ImGui::Text("Metallic");

                ImGui::Image(mat.Roughness->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                ImGui::Text("Roughness");

                ImGui::Image(mat.AO->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                ImGui::Text("AO");

                if (ImGui::Button("Save")) {
                    auto path = Project::GetAssetPath(Project::GetAssetManager().GetAsset(m_Context).FilePath);

                    BL_CORE_TRACE("Saving material {} to {}", m_Context, path.string());
                    Material::Save(mat, path);
                }
            }

            ImGui::End();
        }
    }

    void MaterialEditorPanel::SetContext(u64 materialHandle) {
        m_Context = materialHandle;
    }

} // namespace BlackberryEditor