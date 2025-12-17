#include "material_editor_panel.hpp"

using namespace Blackberry;

namespace BlackberryEditor {

    void MaterialEditorPanel::OnUIRender(bool& open) {
        if (!open) return;

        if (ImGui::Begin("Material Editor", &open)) {
            ImGui::Button(fmt::to_string(m_Context).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0.0f));

            if (ImGui::BeginDragDropTarget()) {
                const ImGuiPayload* assetManagerPayload = ImGui::AcceptDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP");
                const ImGuiPayload* fileBrowserPayload = ImGui::AcceptDragDropPayload("FILE_BROWSER_FILE_DRAG_DROP");

                if (assetManagerPayload) {
                    SetContext(*reinterpret_cast<u64*>(assetManagerPayload->Data));
                } else if (fileBrowserPayload) {
                    // Get asset out of file browser payload
                    FS::Path p = reinterpret_cast<char*>(fileBrowserPayload->Data);

                    if (Project::GetAssetManager().ContainsAsset(p)) {
                        auto& asset = Project::GetAssetManager().GetAssetFromPath(p);
                        if (asset.Type == AssetType::Material) {
                            SetContext(asset.Handle);
                        }
                    }
                }

                ImGui::EndDragDropTarget();
            }

            if (Project::GetAssetManager().ContainsAsset(m_Context)) {
                Material& mat = std::get<Material>(Project::GetAssetManager().GetAsset(m_Context).Data);

                ImGui::Checkbox("Use Albedo Texture", &mat.UseAlbedoTexture);

                if (mat.UseAlbedoTexture) {
                    ImGui::Image(mat.AlbedoTexture->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                    ImGui::Text("Albedo");
                } else {
                    ImGui::ColorEdit4("##AlbedoColor", &mat.AlbedoColor.x); ImGui::SameLine();
                    ImGui::Text("Albedo");
                }

                ImGui::Checkbox("Use Metallic Texture", &mat.UseMetallicTexture);

                if (mat.UseMetallicTexture) {
                    ImGui::Image(mat.MetallicTexture->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                    ImGui::Text("Metallic");
                } else {
                    ImGui::SliderFloat("##MetallicFactor", &mat.MetallicFactor, 0.0f, 1.0f); ImGui::SameLine();
                    ImGui::Text("Metallic");
                }

                ImGui::Checkbox("Use Roughness Texture", &mat.UseRoughnessTexture);

                if (mat.UseRoughnessTexture) {
                    ImGui::Image(mat.RoughnessTexture->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                    ImGui::Text("Roughness");
                } else {
                    ImGui::SliderFloat("##RoughnessFactor", &mat.RoughnessFactor, 0.0f, 1.0f); ImGui::SameLine();
                    ImGui::Text("Roughness");
                }

                ImGui::Checkbox("Use AO Texture", &mat.UseAOTexture);

                if (mat.UseAOTexture) {
                    ImGui::Image(mat.AOTexture->ID, ImVec2(128.0f, 128.0f)); ImGui::SameLine();
                    ImGui::Text("AO");
                } else {
                    ImGui::SliderFloat("##AOFactor", &mat.AOFactor, 0.0f, 1.0f); ImGui::SameLine();
                    ImGui::Text("AO");
                }

                if (ImGui::Button("Save")) {
                    auto path = Project::GetAssetPath(Project::GetAssetManager().GetAsset(m_Context).FilePath);

                    BL_CORE_TRACE("Saving material {} to {}", m_Context, path.String());
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