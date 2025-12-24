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
        Ref<Texture2D> AlbedoTexture = CreateRef<Texture2D>();
        FS::Path AlbedoTexturePath;
        BlVec4 AlbedoColor = BlVec4(0.5f, 0.5f, 0.5f, 1.0f);

        bool UseMetallicTexture = false;
        Ref<Texture2D> MetallicTexture = CreateRef<Texture2D>();
        FS::Path MetallicTexturePath;
        f32 MetallicFactor = 0.0f;

        bool UseRoughnessTexture = false;
        Ref<Texture2D> RoughnessTexture = CreateRef<Texture2D>();
        FS::Path RoughnessTexturePath;
        f32 RoughnessFactor = 1.0f;

        bool UseAOTexture = false;
        Ref<Texture2D> AOTexture = CreateRef<Texture2D>();
        FS::Path AOTexturePath;
        f32 AOFactor = 1.0f;

        u32 ID = 0;
    };

} // namespace Blackberry