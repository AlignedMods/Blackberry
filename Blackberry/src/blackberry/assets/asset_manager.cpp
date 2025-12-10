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

    Asset& AssetManager::GetAsset(AssetHandle handle) {
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
        Ref<Texture2D> tex = Texture2D::Create(path);
        AddAsset(name, {path, AssetType::Texture, tex});
    }

    void AssetManager::AddFontFromPath(const std::string& name, const std::filesystem::path& path) {
        Font font = Font::Create(path);
        AddAsset(name, {path, AssetType::Font, font});
    }

    void AssetManager::AddModelFromPath(const std::string& name, const std::filesystem::path& path) {
        Model model = Model::Create(path);
        AddAsset(name, {path, AssetType::Model, model});
    }

} // namespace Blackberry