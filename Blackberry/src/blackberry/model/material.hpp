#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"

#include <filesystem>

namespace Blackberry {

    // A PBR material which holds a unique id
    struct Material {
        static Material Create(const std::filesystem::path& path);
        static void Save(Material& mat, const std::filesystem::path& path);

        Ref<Texture2D> Albedo;
        Ref<Texture2D> Metallic;
        Ref<Texture2D> Roughness;
        Ref<Texture2D> AO;

        u32 ID = 0;
    };

} // namespace Blackberry