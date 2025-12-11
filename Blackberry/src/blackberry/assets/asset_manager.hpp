#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/scene/uuid.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/font/font.hpp"
#include "blackberry/model/model.hpp"
#include "blackberry/model/material.hpp"
#include "blackberry/core/util.hpp"

#include <filesystem>
#include <unordered_map>
#include <variant>

namespace Blackberry {

    using AssetHandle = u64;

    enum class AssetType {
        Texture = 0,
        Font = 1,
        Model = 2,
        Material = 3
    };

    inline const char* AssetTypeToString(AssetType type) {
        switch (type) {
            case AssetType::Texture: return "Texture"; break;
            case AssetType::Font: return "Font"; break;
            case AssetType::Model: return "Model"; break;
            case AssetType::Material: return "Material"; break;
        }

        BL_ASSERT(false, "Unknown asset type! (memory corruption potential)");
        return "";
    }

    inline AssetType StringToAssetType(const std::string& type) {
        if (type == "Texture") return AssetType::Texture;
        if (type == "Font") return AssetType::Font;
        if (type == "Model") return AssetType::Model;
        if (type == "Material") return AssetType::Material;

        BL_ASSERT(false, "Unknown asset type {}", type);
        return AssetType::Texture;
    }

    struct Asset {
        std::filesystem::path FilePath;
        AssetType Type;
        std::variant<Ref<Texture2D>, Font, Model, Material> Data;
    };

    class AssetManager {
    public:
        using HandleMap = std::unordered_map<AssetHandle, Asset>;

        AssetManager() = default;

        static AssetManager* Copy(AssetManager* current);

        AssetHandle AddAsset(const Asset& asset);
        void AddAssetWithHandle(AssetHandle handle, const Asset& asset);

        const Asset& GetAsset(AssetHandle handle) const;
        Asset& GetAsset(AssetHandle handle);
        const bool ContainsAsset(AssetHandle handle) const;

        const Asset& GetAssetFromPath(const std::filesystem::path& path) const;
        Asset& GetAssetFromPath(const std::filesystem::path& path);
        const bool ContainsAsset(const std::filesystem::path& path);

        AssetHandle GetHandleFromPath(const std::filesystem::path& path) const;

        const HandleMap& GetAllAssets() const;

        // helper functions (all you really need is AddAsset)
        void AddTextureFromPath(const std::filesystem::path& path);
        void AddFontFromPath(const std::filesystem::path& path);
        void AddModelFromPath(const std::filesystem::path& path);

    public:
        std::filesystem::path AssetDirectory;

    private:
        HandleMap m_AssetMap;
        std::unordered_map<std::filesystem::path, AssetHandle> m_AssetHandleMap;
    };

} // namespace Blackberry