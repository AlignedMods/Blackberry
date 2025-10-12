#include "blackberry/assets/asset_manager.hpp"

namespace Blackberry {

    AssetHandle AssetManager::AddAsset(const Asset& asset) {
        AssetHandle handle = UUID();
        AddAssetWithHandle(handle, asset);

        return handle;
    }

    void AssetManager::AddAssetWithHandle(AssetHandle handle, const Asset& asset) {
        m_AssetMap[handle] = asset;
    }

    Asset& AssetManager::GetAsset(AssetHandle handle) {
        return m_AssetMap.at(handle);
    }

    const std::unordered_map<AssetHandle, Asset>& AssetManager::GetAllAssets() const {
        return m_AssetMap;
    }

    void AssetManager::AddTextureFromPath(const std::string& name, const std::filesystem::path& path) {
        std::filesystem::path relative = path;
        
        if (path.is_absolute()) {
            relative = std::filesystem::relative(path, std::filesystem::current_path());
        }

        Blackberry::Image image(relative);
        BlTexture tex;
        tex.Create(image);

        AddAsset({relative, AssetType::Texture, tex});
    }

} // namespace Blackberry