#pragma once

#include "blackberry/core/types.hpp"
// #include "blackberry/rendering/texture.hpp"

#include <filesystem>

namespace Blackberry {

    enum class ImageFormat {
        U8,
        RGB8,
        RGBA8,
        F32,
        RGB32F,
        RGBA32F
    };

    class Image {
    public:
        static Image Create(const std::filesystem::path& path);
        static Image Create(const void* data, u32 width, u32 height, ImageFormat format);

        void WriteOut(const std::string& fileName);

    public:
        void* Data = nullptr;
        i32 Width = 0, Height = 0;
        i32 Channels = 0;
        ImageFormat Format = ImageFormat::U8;
    };

} // namespace Blackberry