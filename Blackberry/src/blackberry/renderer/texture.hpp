#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/memory.hpp"
#include "blackberry/renderer/image.hpp"

namespace Blackberry {

    enum class TextureFormat {
        U8,

        RGB8,
        RGBA8,

        RG16F,
        RGB16F,
        RGBA16F
    };

    enum class TextureFiltering { Nearest, Linear };

    enum class FramebufferAttachmentType {
        // Color attachments
        ColorR8,
        ColorRGBA8,
        ColorRGBA16F,
        ColorR32I,
        ColorR32F,

        // Depth attachments
        Depth,
        Depth24
    };

    struct RenderTextureAttachment {
        u32 Attachment = 0;
        FramebufferAttachmentType Type;
    };

    struct FramebufferSpecification {
        u32 Width = 0;
        u32 Height = 0;
        std::vector<RenderTextureAttachment> Attachments;
        std::vector<u32> ActiveAttachments;
    };

    // Base class for all textures (Texture2D, TextureCubemap, ...)
    struct Texture {
        virtual ~Texture() = default;

        u32 ID = 0;
        u64 BindlessHandle = 0;
        u32 Width = 0;
        u32 Height = 0;
        TextureFormat Format = TextureFormat::RGBA8;
    };

    struct Texture2D : public Texture {
        Texture2D() = default;
        ~Texture2D();

        static Ref<Texture> Create(u32 width, u32 height);
        static Ref<Texture> Create(const FS::Path& path, TextureFormat desiredFormat = TextureFormat::RGBA8);
        static Ref<Texture> Create(Ref<Image> image);
        static Ref<Texture> Create(void* pixels, u32 width, u32 height, TextureFormat pixelFormat, TextureFiltering filter = TextureFiltering::Linear);
    
        void* ReadPixels();
    };

    struct TextureCubemap : public Texture {
        TextureCubemap() = default;
        ~TextureCubemap();

        static Ref<Texture> Create(u32 width, u32 height, TextureFormat desiredFormat = TextureFormat::RGBA8);
    };
    
    struct Framebuffer {
        Framebuffer() = default;
        ~Framebuffer();

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
        void Delete();
        void Resize(u32 width, u32 height);

        void ClearAttachmentInt(u32 attachment, int value);
        void ClearAttachmentFloat(u32 attachment, f32 value);

        // NOTE: You must free the memory that comes with this function! (with free() NOT delete!)
        void* ReadPixels(u32 attachment, BlVec2 position, BlVec2 dimensions, u32 size);

        void BlitDepthBuffer(Ref<Framebuffer> other);

        void AttachColorAttachment(u32 attachment, const Ref<Texture>& texture, u32 mip);
        void AttachColorAttachmentCubemap(u32 attachment, const Ref<Texture>& texture, u32 side, u32 mip);
    
        u32 ID = 0;
        FramebufferSpecification Specification;
        std::vector<Ref<Texture>> Attachments;

    private:
        void Invalidate();
    };

} // namespace Blackberry