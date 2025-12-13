#include "blackberry/image/image.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/application/renderer.hpp"
#include "blackberry/application/application.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace Blackberry {

    Image Image::Create(const FS::Path& path) {
        Image im;

        im.Data = stbi_load(path.CString(), &im.Width, &im.Height, &im.Channels, 4);
        im.Format = ImageFormat::RGBA8;

        if (!im.Data) {
            BL_CORE_ERROR("Failed to load image {}!", path.String());
        }

        return im;
    }

    Image Image::Create(const void* data, u32 width, u32 height, ImageFormat format) {
        u32 size = 4;

        switch (format) {
            case ImageFormat::U8:
                size = 1;
                break;
            case ImageFormat::RGB8:
                size = 3;
                break;
            case ImageFormat::RGBA8:
                size = 4;
                break;
        }

        Image im;
        im.Data = new u8[width * height * size];
        memcpy(im.Data, data, width * height * size);

        im.Width = width;
        im.Height = height;
        im.Format = format;

        return im;
    }

    void Image::WriteOut(const std::string& fileName) {
        stbi_write_png(fileName.c_str(), Width, Height, 3, Data, Width * 3);
    }

} // namespace Blackberry
