#pragma once

#include "blackberry/assets/asset_manager.hpp"

namespace Blackberry {

    class AssetSerializer {
    public:
        AssetSerializer() = default;
        AssetSerializer(AssetManager* assetManager);

        void Serialize(const std::filesystem::path& path);
        void Deserialize(const std::filesystem::path& path);

    private:
        AssetManager* m_AssetManager = nullptr;
    };

} // namespace Blackberry