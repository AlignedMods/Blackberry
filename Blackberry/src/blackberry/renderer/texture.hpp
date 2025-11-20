#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/image/image.hpp"

namespace Blackberry {

    enum class TextureFiltering { Nearest, Linear };

    struct Texture2D {
        Texture2D() = default;

        static Texture2D Create(u32 width, u32 height);
        static Texture2D Create(const std::filesystem::path& path);
        static Texture2D Create(const Image& image);
        static Texture2D Create(void* pixels, u32 width, u32 height, ImageFormat pixelFormat, TextureFiltering filter = TextureFiltering::Nearest);

        void Delete();
    
        void* ReadPixels();
    
        u32 ID = 0;
        BlVec2<u32> Size;
        ImageFormat Format = ImageFormat::RGBA8;
    };
    
    struct RenderTexture {
        RenderTexture() = default;

        static RenderTexture Create(u32 width, u32 height);
        void Delete();
    
        u32 ID = 0;
        BlVec2<u32> Size;
        Texture2D ColorAttachment;
        Texture2D DepthAttachment;
    };

} // namespace Blackberry