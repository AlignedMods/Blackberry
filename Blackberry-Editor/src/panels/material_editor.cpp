#include "material_editor.hpp"

using namespace Blackberry;

namespace BlackberryEditor {

    void MaterialEditor::OnUIRender(bool& open) {
        if (!open) return;

        if (ImGui::Begin("Material Editor", &open)) {
            if (Project::GetAssetManager().ContainsAsset(m_Context)) {
                Material& mat = std::get<Material>(Project::GetAssetManager().GetAsset(m_Context).Data);

                // ImGui::ColorEdit3("Albedo", &mat.Albedo.x);
                // ImGui::SliderFloat("Metallic", &mat.Metallic, 0.0f, 1.0f);
                // ImGui::SliderFloat("Roughness", &mat.Roughness, 0.0f, 1.0f);
                // ImGui::SliderFloat("AO", &mat.AO, 0.0f, 1.0f);

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