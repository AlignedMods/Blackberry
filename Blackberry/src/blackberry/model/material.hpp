#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/core/path.hpp"

namespace Blackberry {

    // A PBR material which holds a unique id
    struct Material {
        static Material Create();
        static Material Create(const FS::Path& path);
        static void Save(Material& mat, const FS::Path& path);

        bool UseAlbedoTexture = false;
        Ref<Texture> AlbedoTexture = CreateRef<Texture>();
        FS::Path AlbedoTexturePath;
        BlVec4 AlbedoColor = BlVec4(0.5f, 0.5f, 0.5f, 1.0f);

        bool UseMetallicTexture = false;
        Ref<Texture> MetallicTexture = CreateRef<Texture>();
        FS::Path MetallicTexturePath;
        f32 MetallicFactor = 0.0f;

        bool UseRoughnessTexture = false;
        Ref<Texture> RoughnessTexture = CreateRef<Texture>();
        FS::Path RoughnessTexturePath;
        f32 RoughnessFactor = 1.0f;

        bool UseAOTexture = false;
        Ref<Texture> AOTexture = CreateRef<Texture>();
        FS::Path AOTexturePath;
        f32 AOFactor = 1.0f;

        f32 Emission = 0.0f;

        u32 ID = 0;
    };

} // namespace Blackberry