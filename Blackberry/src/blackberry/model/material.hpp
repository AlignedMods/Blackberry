#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"

#include <filesystem>

namespace Blackberry {

    // A PBR material which holds a unique id
    struct Material {
        static Material Create(const std::filesystem::path& path);
        static void Save(Material& mat, const std::filesystem::path& path);

        Texture2D Albedo;
        Texture2D Metallic;
        Texture2D Roughness;
        Texture2D AO;

        u32 ID = 0;
    };

} // namespace Blackberry