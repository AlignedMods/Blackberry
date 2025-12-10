#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/image/image.hpp"
#include "blackberry/core/memory.hpp"

namespace Blackberry {

    enum class TextureFiltering { Nearest, Linear };

    enum class RenderTextureAttachmentType {
        // Color attachments
        ColorRGBA8,
        ColorRGBA16F,

        // Depth attachments
        Depth
    };

    struct RenderTextureAttachment {
        u32 Attachment = 0;
        RenderTextureAttachmentType Type;
    };

    struct RenderTextureSpecification {
        BlVec2<u32> Size;
        std::vector<RenderTextureAttachment> Attachments;
        std::vector<u32> ActiveAttachments;
    };

    struct Texture2D {
        Texture2D() = default;
        ~Texture2D();

        static Ref<Texture2D> Create(u32 width, u32 height);
        static Ref<Texture2D> Create(const std::filesystem::path& path);
        static Ref<Texture2D> Create(const Image& image);
        static Ref<Texture2D> Create(void* pixels, u32 width, u32 height, ImageFormat pixelFormat, TextureFiltering filter = TextureFiltering::Nearest);

        void Delete();
    
        void* ReadPixels();
    
        u32 ID = 0;
        u64 BindlessHandle = 0; // for bindless textures
        BlVec2<u32> Size;
        ImageFormat Format = ImageFormat::RGBA8;
    };
    
    struct RenderTexture {
        RenderTexture() = default;
        ~RenderTexture();

        static Ref<RenderTexture> Create(const RenderTextureSpecification& spec);
        void Delete();
        void Resize(BlVec2<u32> size);
    
        u32 ID = 0;
        RenderTextureSpecification Specification;
        std::vector<Ref<Texture2D>> Attachments;
    };

} // namespace Blackberry