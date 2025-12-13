#include "blackberry/assets/asset_manager.hpp"
#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    AssetManager* AssetManager::Copy(AssetManager* current) {
        AssetManager* newAssetManager = new AssetManager();

        for (const auto&[handle, asset] : current->m_AssetMap) {
            newAssetManager->AddAssetWithHandle(handle, asset);
        }

        return newAssetManager;
    }

    AssetHandle AssetManager::AddAsset(const Asset& asset) {
        AssetHandle handle = UUID();
        AddAssetWithHandle(handle, asset);

        return handle;
    }

    void AssetManager::AddAssetWithHandle(AssetHandle handle, const Asset& asset) {
        m_AssetMap[handle] = asset;
        m_AssetHandleMap[asset.FilePath] = handle;
    }

    const Asset& AssetManager::GetAsset(AssetHandle handle) const {
        return m_AssetMap.at(handle);
    }

    Asset& AssetManager::GetAsset(AssetHandle handle) {
        return m_AssetMap.at(handle);
    }

    const bool AssetManager::ContainsAsset(AssetHandle handle) const {
        return m_AssetMap.contains(handle);
    }

    const Asset& AssetManager::GetAssetFromPath(const FS::Path& path) const {
        return m_AssetMap.at(m_AssetHandleMap.at(path));
    }

    Asset& AssetManager::GetAssetFromPath(const FS::Path& path) {
        return m_AssetMap.at(m_AssetHandleMap.at(path));
    }

    const bool AssetManager::ContainsAsset(const FS::Path& path) {
        return m_AssetHandleMap.contains(path);
    }

    AssetHandle AssetManager::GetHandleFromPath(const FS::Path& path) const {
        return m_AssetHandleMap.at(path);
    }

    const AssetManager::HandleMap& AssetManager::GetAllAssets() const {
        return m_AssetMap;
    }

    void AssetManager::AddTextureFromPath(const FS::Path& path) {
        Ref<Texture2D> tex = Texture2D::Create(path);
        AddAsset({path, AssetType::Texture, tex});
    }

    void AssetManager::AddFontFromPath(const FS::Path& path) {
        Font font = Font::Create(path);
        AddAsset({path, AssetType::Font, font});
    }

    void AssetManager::AddModelFromPath(const FS::Path& path) {
        Model model = Model::Create(path);
        AddAsset({path, AssetType::Model, model});
    }

} // namespace Blackberry