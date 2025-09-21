#include "blackberry/image/image.hpp"
#include "blackberry/log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Blackberry {

    Image LoadImageFromFile(const std::filesystem::path& path) {
        Image im{};

        stbi_set_flip_vertically_on_load(false);
        im.data = stbi_load(path.string().c_str(), &im.width, &im.height, &im.channels, 4);

        if (!im.data) {
            Log(Log_Error, "Failed to load image!");
        }

        return im;
    }

    void UnloadImage(const Image& image) {
        stbi_image_free((void*)image.data);
    }

} // namespace Blackberry
