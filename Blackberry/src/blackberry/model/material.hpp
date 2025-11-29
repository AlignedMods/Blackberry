#pragma once

#include "blackberry/core/types.hpp"

#include <filesystem>

namespace Blackberry {

    struct Material {
        static Material Create(const std::filesystem::path& path);
        static void Save(Material& mat, const std::filesystem::path& path);

        u64 Diffuse = 0;
        u64 Specular = 0;
        f32 Shininess = 32.0f;

        u32 ID = 0;
    };

} // namespace Blackberry