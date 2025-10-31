#include "blackberry/assets/asset_manager.hpp"

namespace Blackberry {

    AssetManager* AssetManager::Copy(AssetManager* current) {
        AssetManager* newAssetManager = new AssetManager();

        for (const auto&[handle, asset] : current->m_AssetMap) {
            newAssetManager->AddAssetWithHandle(handle, asset);
        }

        return newAssetManager;
    }

    AssetHandle AssetManager::AddAsset(const std::string& name, const Asset& asset) {
        AssetHandle handle = UUID();
        AddAssetWithHandle(handle, asset);

        m_AssetHandleMap[name] = handle;

        return handle;
    }

    void AssetManager::AddAssetWithHandle(AssetHandle handle, const Asset& asset) {
        m_AssetMap[handle] = asset;
    }

    const Asset& AssetManager::GetAsset(AssetHandle handle) const {
        return m_AssetMap.at(handle);
    }

    const bool AssetManager::ContainsAsset(AssetHandle handle) const {
        return m_AssetMap.contains(handle);
    }

    AssetHandle AssetManager::GetAssetHandle(const std::string& name) const {
        return m_AssetHandleMap.at(name);
    }

    const AssetManager::HandleMap& AssetManager::GetAllAssets() const {
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

        AddAsset(name, {relative, AssetType::Texture, tex});
    }

} // namespace Blackberry