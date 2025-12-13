#pragma once

#include "blackberry/assets/asset_manager.hpp"

namespace Blackberry {

    class AssetSerializer {
    public:
        AssetSerializer() = default;
        AssetSerializer(AssetManager* assetManager);

        void Serialize(const FS::Path& path);
        void Deserialize(const FS::Path& path);

    private:
        AssetManager* m_AssetManager = nullptr;
    };

} // namespace Blackberry