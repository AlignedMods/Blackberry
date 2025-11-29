#include "material_editor.hpp"

using namespace Blackberry;

namespace BlackberryEditor {

    void MaterialEditor::OnUIRender(bool& open) {
        if (!open) return;

        if (ImGui::Begin("Material Editor", &open)) {
            if (Project::GetAssetManager().ContainsAsset(m_Context)) {
                Material& mat = std::get<Material>(Project::GetAssetManager().GetAsset(m_Context).Data);

                if (Project::GetAssetManager().ContainsAsset(mat.Diffuse)) {
                    Texture2D diffuse = std::get<Texture2D>(Project::GetAssetManager().GetAsset(mat.Diffuse).Data);

                    ImGui::Image(diffuse.ID, ImVec2(64.0f, 64.0f));

                    if (ImGui::BeginItemTooltip()) {
                        ImGui::Image(diffuse.ID, ImVec2(256.0f, 256.0f));
                    
                        ImGui::EndTooltip();
                    }
                }

                if (Project::GetAssetManager().ContainsAsset(mat.Specular)) {
                    Texture2D specular = std::get<Texture2D>(Project::GetAssetManager().GetAsset(mat.Specular).Data);

                    ImGui::Image(specular.ID, ImVec2(64.0f, 64.0f));

                    if (ImGui::BeginItemTooltip()) {
                        ImGui::Image(specular.ID, ImVec2(256.0f, 256.0f));
                    
                        ImGui::EndTooltip();
                    }
                }

                ImGui::SliderFloat("Shininess", &mat.Shininess, 1.0f, 512.0f, nullptr, ImGuiSliderFlags_Logarithmic);

                if (ImGui::Button("Save")) {
                    auto path = Project::GetAssetPath(Project::GetAssetManager().GetAsset(m_Context).FilePath);

                    BL_CORE_TRACE("Saving material {} to {}", m_Context, path.string());
                    Material::Save(mat, path);
                }
            }

            ImGui::End();
        }
    }

    void MaterialEditor::SetContext(u64 materialHandle) {
        m_Context = materialHandle;
    }

} // namespace BlackberryEditor