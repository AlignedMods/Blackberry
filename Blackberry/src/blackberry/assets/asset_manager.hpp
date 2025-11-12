#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/scene/uuid.hpp"
#include "blackberry/rendering/texture.hpp"

#include <filesystem>
#include <unordered_map>
#include <variant>

namespace Blackberry {

    using AssetHandle = u64;

    enum class AssetType {
        Texture
    };

    struct Asset {
        std::filesystem::path FilePath;
        AssetType Type;
        std::variant<BlTexture> Data;
    };

    class AssetManager {
    public:
        using HandleMap = std::unordered_map<AssetHandle, Asset>;

        AssetManager() = default;

        static AssetManager* Copy(AssetManager* current);

        AssetHandle AddAsset(const std::string& name, const Asset& asset);
        void AddAssetWithHandle(AssetHandle handle, const Asset& asset);

        const Asset& GetAsset(AssetHandle handle) const;
        const bool ContainsAsset(AssetHandle handle) const;
        AssetHandle GetAssetHandle(const std::string& name) const;
        const HandleMap& GetAllAssets() const;

        // helper functions (all you really need is AddAsset)
        void AddTextureFromPath(const std::string& name, const std::filesystem::path& path);

    private:
        HandleMap m_AssetMap;
        std::unordered_map<std::string, AssetHandle> m_AssetHandleMap;
    };

} // namespace Blackberry