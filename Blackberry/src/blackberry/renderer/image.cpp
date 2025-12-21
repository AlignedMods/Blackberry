#include "blackberry/renderer/image.hpp"

#include "stb_image.h"

namespace Blackberry {

    Ref<Image> Image::Create(const FS::Path& path, bool loadAsFloat) {
        int width, height, channels;
        void* pixels = nullptr;
        ImageFormat format = ImageFormat::RGB8;

        if (loadAsFloat) {
            pixels = stbi_loadf(path.String().c_str(), &width, &height, &channels, 0);
            
            switch (channels) {
                case 3: format = ImageFormat::RGB32F; break;
                case 4: format = ImageFormat::RGBA32F; break;
                default: BL_CORE_WARN("Unknown channel count!"); break;
            }
        } else {
            pixels = stbi_load(path.String().c_str(), &width, &height, &channels, 0);

            switch (channels) {
                case 3: format = ImageFormat::RGB8; break;
                case 4: format = ImageFormat::RGBA8; break;
                default: BL_CORE_WARN("Unknown channel count!"); break;
            }
        }

        Ref<Image> im = Create(pixels, width, height, format);
        stbi_image_free(pixels); // We can safely free pixels since Create allocates new buffer

        return im;
    }

    Ref<Image> Image::Create(const void* pixels, u32 width, u32 height, ImageFormat format) {
        Ref<Image> im = CreateRef<Image>();

        u32 pixelSizeBytes = 4;

        switch (format) {
            case ImageFormat::U8: pixelSizeBytes = 1; break;
            case ImageFormat::RGB8: pixelSizeBytes = 3; break;
            case ImageFormat::RGBA8: pixelSizeBytes = 4; break;
            case ImageFormat::RGB32F: pixelSizeBytes = sizeof(f32) * 3; break;
            case ImageFormat::RGBA32F: pixelSizeBytes = sizeof(f32) * 4; break;
        }

        im->Pixels = malloc(width * height * pixelSizeBytes); // We use malloc here because we don't know the type of pixels, only the size

        // copy pixels
        memcpy(im->Pixels, pixels, width * height * pixelSizeBytes);

        im->Width = width;
        im->Height = height;
        im->Format = format;

        return im;
    }

    Image::~Image() {
        free(Pixels);
        Pixels = nullptr;
        Width = 0;
        Height = 0;
    }

} // namespace Blackberry