#include "blackberry/assets/asset_serializer.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    static const char* AssetTypeToString(AssetType type) {
        switch (type) {
            case AssetType::Texture: return "Texture"; break;
            case AssetType::Font: return "Font"; break;
            case AssetType::Model: return "Model"; break;
            case AssetType::Material: return "Material"; break;
        }

        BL_ASSERT(false, "Unknown asset type! (memory corruption potential)");
        return "";
    }

    static AssetType StringToAssetType(const std::string& type) {
        if (type == "Texture") return AssetType::Texture;
        if (type == "Font") return AssetType::Font;
        if (type == "Model") return AssetType::Model;
        if (type == "Material") return AssetType::Material;

        BL_ASSERT(false, "Unknown asset type {}", type);
        return AssetType::Texture;
    }

    AssetSerializer::AssetSerializer(AssetManager* assetManager)
        : m_AssetManager(assetManager) {}

    void AssetSerializer::Serialize(const std::filesystem::path& path) {
        auto& assets = m_AssetManager->GetAllAssets();
        if (assets.size() == 0) return;

        json j;

        for (auto&[handle, asset] : assets) {
            std::string name = "Handle - " + std::to_string(handle);

            j["Assets"][name] = {
                {"Handle", handle},
                {"Path", asset.FilePath},
                {"Type", AssetTypeToString(asset.Type)}
            };
        }

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void AssetSerializer::Deserialize(const std::filesystem::path& path) {
        std::string contents = ReadEntireFile(path);

        json j = json::parse(contents);
        auto& assets = j.at("Assets");

        for (auto& jsonAsset : assets) {
            Asset asset;
            asset.Type = StringToAssetType(jsonAsset.at("Type"));

            std::filesystem::path path = jsonAsset.at("Path");
            asset.FilePath = path;
            
            if (asset.Type == AssetType::Texture) {
                Texture2D tex = Texture2D::Create(Project::GetAssetPath(path));
                asset.Data = tex;
            } else if (asset.Type == AssetType::Font) {
                Font font = Font::Create(Project::GetAssetPath(path));
                asset.Data = font;
            } else if (asset.Type == AssetType::Model) {
                Model model = Model::Create(Project::GetAssetPath(path));
                asset.Data = model;
            } else if (asset.Type == AssetType::Material) {
                Material material = Material::Create(Project::GetAssetPath(path));
                asset.Data = material;
            }

            m_AssetManager->AddAssetWithHandle(jsonAsset.at("Handle"), asset);
        }
    }

} // namespace Blackberry