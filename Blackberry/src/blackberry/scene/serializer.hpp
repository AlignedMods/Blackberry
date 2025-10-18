#pragma once

#include "blackberry/scene/scene.hpp"

namespace Blackberry {

    class SceneSerializer {
    public:
        SceneSerializer() = default;
        SceneSerializer(Scene* scene, const std::filesystem::path& assetDirectory);

        void Serialize(const std::filesystem::path& path);
        void Deserialize(const std::filesystem::path& path);

    private:
        Scene* m_Scene = nullptr;
        std::filesystem::path m_AssetDirectory;
    };

} // namespace Blackberry