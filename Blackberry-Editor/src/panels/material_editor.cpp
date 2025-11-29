#include "material_editor.hpp"

using namespace Blackberry;

namespace BlackberryEditor {

    void MaterialEditor::OnUIRender(bool& open) {
        if (!open) return;

        if (ImGui::Begin("Material Editor", &open)) {
            if (Project::GetAssetManager().ContainsAsset(m_Context)) {
                Material& mat = std::get<Material>(Project::GetAssetManager().GetAsset(m_Context).Data);

                ImGui::ColorEdit3("Ambient", &mat.Ambient.x);
                ImGui::ColorEdit3("Diffuse", &mat.Diffuse.x);
                ImGui::ColorEdit3("Specular", &mat.Specular.x);

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