#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/ecs/components.hpp"
#include "blackberry/scene/uuid.hpp"

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
        AssetManager() = default;

        AssetHandle AddAsset(const Asset& asset);
        void AddAssetWithHandle(AssetHandle handle, const Asset& asset);

        Asset& GetAsset(AssetHandle handle);
        const std::unordered_map<AssetHandle, Asset>& GetAllAssets() const;

        // helper functions (all you really need is AddAsset)
        void AddTextureFromPath(const std::string& name, const std::filesystem::path& path);

    private:
        std::unordered_map<AssetHandle, Asset> m_AssetMap;
    };

} // namespace Blackberry