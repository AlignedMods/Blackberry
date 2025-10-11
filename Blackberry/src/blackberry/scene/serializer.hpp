#pragma once

#include "blackberry/scene/scene.hpp"

namespace Blackberry {

    class Serializer {
    public:
        Serializer() = default;
        Serializer(Scene* scene);

        void Serialize(const std::filesystem::path& path);
        void Deserialize(const std::filesystem::path& path);

    private:
        Scene* m_Scene = nullptr;
    };

} // namespace Blackberry