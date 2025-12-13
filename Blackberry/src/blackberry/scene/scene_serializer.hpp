#pragma once

#include "blackberry/scene/scene.hpp"
#include "blackberry/core/path.hpp"

namespace Blackberry {

    class SceneSerializer {
    public:
        SceneSerializer() = default;
        SceneSerializer(Scene* scene);

        void Serialize(const FS::Path& path);
        void Deserialize(const FS::Path& path);

    private:
        Scene* m_Scene = nullptr;
    };

} // namespace Blackberry