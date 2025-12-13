#include "asset_manager_panel.hpp"

#include "blackberry.hpp"

using namespace Blackberry;

namespace BlackberryEditor {

    void AssetManagerPanel::OnUIRender(bool& open) {
        if (!open) return;

        ImGui::Begin("Asset Manager", &open);

        auto& assetManager = Project::GetAssetManager();

        ImGui::Checkbox("Allow Editing", &m_AllowEditing);

        if (ImGui::CollapsingHeader("Registry")) {
            for (auto&[handle, asset] : assetManager.GetAllAssets()) {
                ImGui::SeparatorText("Asset");
                ImGui::Indent();

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                ImGui::PushID(&handle);

                ImGui::Text("Handle: "); ImGui::SameLine(150.0f);
                ImGui::Button(fmt::to_string(handle).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0.0f));

                if (ImGui::BeginDragDropSource()) {
                    ImGui::SetDragDropPayload("ASSET_MANAGER_HANDLE_DRAG_DROP", &handle, sizeof(handle));
                    ImGui::Text(fmt::to_string(handle).c_str());
                    ImGui::EndDragDropSource();
                }
                ImGui::Separator();

                ImGui::Text("Path: "); ImGui::SameLine(150.0f);
                ImGui::Button(asset.FilePath.CString(), ImVec2(ImGui::GetContentRegionAvail().x, 0.0f));
                ImGui::Separator();

                ImGui::Text("Type: "); ImGui::SameLine(150.0f);
                ImGui::Button(AssetTypeToString(asset.Type), ImVec2(ImGui::GetContentRegionAvail().x, 0.0f));

                ImGui::PopID();
                ImGui::PopStyleVar();

                ImGui::Unindent();
            }
        }

        ImGui::End();
    }

} // namespace BlackberryEditor