#include "blackberry/assets/asset_manager.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/project/project.hpp"

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
        m_AssetMap[handle].Handle = handle;
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

    void AssetManager::LoadTextureFromPath(const FS::Path& path) {
        FS::Path full = Project::GetAssetPath(path);
        Ref<Texture2D> tex = Texture2D::Create(full);
        AddAsset({path, AssetType::Texture, tex});
    }

    void AssetManager::LoadFontFromPath(const FS::Path& path) {
        FS::Path full = Project::GetAssetPath(path);
        Font font = Font::Create(full);
        AddAsset({path, AssetType::Font, font});
    }

    void AssetManager::LoadModelFromPath(const FS::Path& path) {
        FS::Path full = Project::GetAssetPath(path);
        Model model = Model::Create(full);
        AddAsset({path, AssetType::Model, model});
    }

    void AssetManager::LoadMaterialFromPath(const FS::Path& path) {
        FS::Path full = Project::GetAssetPath(path);
        Material mat = Material::Create(full);
        AddAsset({path, AssetType::Material, mat});
    }

    void AssetManager::LoadEnviromentMapFromPath(const FS::Path& path) {
        FS::Path full = Project::GetAssetPath(path);
        Ref<EnviromentMap> env = EnviromentMap::Create(full);
        AddAsset({path, AssetType::EnviromentMap, env});
    }

    void AssetManager::LoadAssetFromPath(const FS::Path& path) {
        std::string ext = path.Extension();
        std::transform(ext.begin(), ext.end(), ext.begin(),
            [](u8 c){ return std::tolower(c); });

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
            LoadTextureFromPath(path);
        } else if (ext == ".ttf" || ext == ".otf") {
            LoadFontFromPath(path);
        } else if (ext == ".glb" || ext == ".gltf") {
            LoadModelFromPath(path);
        } else if (ext == ".blmat") {
            LoadMaterialFromPath(path);
        } else if (ext == ".hdr" || ext == ".hdri") {
            LoadEnviromentMapFromPath(path);
        }
    }

} // namespace Blackberry