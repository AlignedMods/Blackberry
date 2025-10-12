#pragma once

#include "blackberry/assets/asset_manager.hpp"

#include <filesystem>

namespace Blackberry {

    class AssetManagerSerializer {
    public:
        AssetManagerSerializer() = default;
        AssetManagerSerializer(AssetManager* assetManager);

        void Serialize(const std::filesystem::path& path);
        void Deserialize(const std::filesystem::path& path);

    private:
        AssetManager* m_AssetManager = nullptr;
    };

} // namespace Blackberry