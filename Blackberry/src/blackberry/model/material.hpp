#pragma once

#include "blackberry/core/types.hpp"

#include <filesystem>

namespace Blackberry {

    // A PBR material which holds a unique id
    struct Material {
        static Material Create(const std::filesystem::path& path);
        static void Save(Material& mat, const std::filesystem::path& path);

        BlVec3<f32> Albedo;
        f32 Metallic = 0.0;
        f32 Roughness = 0.0f;
        f32 AO = 0.0f;

        u32 ID = 0;
    };

} // namespace Blackberry