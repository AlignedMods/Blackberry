#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"

#include <filesystem>

namespace Blackberry {

    // A PBR material which holds a unique id
    struct Material {
        static Material Create(const FS::Path& path);
        static void Save(Material& mat, const FS::Path& path);

        bool UseAlbedoTexture = false;
        Ref<Texture2D> AlbedoTexture;
        BlVec4<f32> AlbedoColor;

        bool UseMetallicTexture = false;
        Ref<Texture2D> MetallicTexture;
        f32 MetallicFactor = 0.0f;

        bool UseRoughnessTexture = false;
        Ref<Texture2D> RoughnessTexture;
        f32 RoughnessFactor = 0.0f;

        bool UseAOTexture = false;
        Ref<Texture2D> AOTexture;
        f32 AOFactor = 0.0f;

        u32 ID = 0;
    };

} // namespace Blackberry