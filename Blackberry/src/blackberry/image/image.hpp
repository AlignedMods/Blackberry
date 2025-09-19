#pragma once

#include "blackberry/types.hpp"

#include <filesystem>

namespace Blackberry {

    struct Image {
        const void* data;
        i32 width, height;
        i32 channels;
    };

    Image LoadImageFromFile(const std::filesystem::path& path);
    void UnloadImage(const Image& image);

} // namespace Blackberry