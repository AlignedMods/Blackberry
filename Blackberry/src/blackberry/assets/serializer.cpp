#include "blackberry/assets/serializer.hpp"
#include "blackberry/core/log.hpp"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>

namespace Blackberry {

    AssetManagerSerializer::AssetManagerSerializer(AssetManager* assetManager)
        : m_AssetManager(assetManager) {}

    void AssetManagerSerializer::Serialize(const std::filesystem::path& path) {
        json j;

        const auto& assets = m_AssetManager->GetAllAssets();

        for (const auto&[handle, asset] : assets) {
            std::string name = "Handle - " + std::to_string(handle);

            j["Assets"][name]["Handle"] = handle;
            j["Assets"][name]["FilePath"] = asset.FilePath;

            if (asset.Type == AssetType::Texture) {
                j["Assets"][name]["Type"] = "Texture";
            }
        }

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void AssetManagerSerializer::Deserialize(const std::filesystem::path& path) {
        std::ifstream stream(path);
        std::stringstream ss;
        ss << stream.rdbuf();
        std::string contents = ss.str();
        ss.flush();

        json j = json::parse(contents);
        auto& assets = j.at("Assets");

        for (auto& asset : assets) {
            u64 handle = asset.at("Handle");
            std::filesystem::path path(static_cast<std::string>(asset.at("FilePath")));
            Image image(path);
            BlTexture tex;
            tex.Create(image);
            tex.Handle = handle;

            m_AssetManager->AddAssetWithHandle(handle, {path, AssetType::Texture, tex});
            // BL_INFO("Handle: {}", handle);
        }
    }

} // namespace Blackberry