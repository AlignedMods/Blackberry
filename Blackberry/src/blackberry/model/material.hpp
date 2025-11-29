#pragma once

#include "blackberry/core/types.hpp"

#include <filesystem>

namespace Blackberry {

    struct Material {
        static Material Create(const std::filesystem::path& path);
        static void Save(Material& mat, const std::filesystem::path& path);

        BlVec3<f32> Ambient;
        BlVec3<f32> Diffuse;
        BlVec3<f32> Specular;
        f32 Shininess = 32.0f;

        u32 ID = 0;
    };

} // namespace Blackberry