#include "blackberry/assets/asset_serializer.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    AssetSerializer::AssetSerializer(AssetManager* assetManager)
        : m_AssetManager(assetManager) {}

    void AssetSerializer::Serialize(const FS::Path& path) {
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

    void AssetSerializer::Deserialize(const FS::Path& path) {
        std::string contents = Util::ReadEntireFile(path);

        json j = json::parse(contents);
        auto& assets = j.at("Assets");

        for (auto& jsonAsset : assets) {
            Asset asset;
            asset.Type = StringToAssetType(jsonAsset.at("Type"));

            std::string assetPath = jsonAsset.at("Path");
            asset.FilePath = assetPath;
            
            if (asset.Type == AssetType::Texture) {
                Ref<Texture> tex = Texture2D::Create(Project::GetAssetPath(assetPath));
                asset.Data = tex;
            } else if (asset.Type == AssetType::Font) {
                Font font = Font::Create(Project::GetAssetPath(assetPath));
                asset.Data = font;
            } else if (asset.Type == AssetType::Model) {
                Model model = Model::Create(Project::GetAssetPath(assetPath));
                asset.Data = model;
            } else if (asset.Type == AssetType::Material) {
                Material material = Material::Create(Project::GetAssetPath(assetPath));
                asset.Data = material;
            } else if (asset.Type == AssetType::EnviromentMap) {
                Ref<EnviromentMap> env = EnviromentMap::Create(Project::GetAssetPath(assetPath));
                asset.Data = env;
            } else if (asset.Type == AssetType::Scene) {
                Ref<Scene> scene = Scene::Create(Project::GetAssetPath(assetPath));
                asset.Data = scene;
            }

            m_AssetManager->AddAssetWithHandle(jsonAsset.at("Handle"), asset);
        }
    }

} // namespace Blackberry