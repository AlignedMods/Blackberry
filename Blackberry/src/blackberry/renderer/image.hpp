#pragma once

#include "blackberry/core/memory.hpp"
#include "blackberry/core/path.hpp"

namespace Blackberry {

    enum class ImageFormat {
        U8,

        RGB8,
        RGBA8,

        RGB32F,
        RGBA32F
    };

    // Image which gets stored in normal RAM instead of VRAM allowing for editing from the cpu
    struct Image {
        static Ref<Image> Create(const FS::Path& path, bool loadAsFloat = false);
        static Ref<Image> Create(const void* pixels, u32 width, u32 height, ImageFormat format);
        ~Image();

        void* Pixels = nullptr;
        u32 Width = 0;
        u32 Height = 0;
        ImageFormat Format = ImageFormat::RGB8;
    };

} // namespace Blackberry