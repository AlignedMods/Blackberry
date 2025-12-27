#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/memory.hpp"
#include "blackberry/renderer/image.hpp"

namespace Blackberry {

    enum class TextureFormat {
        U8,

        RGB8,
        RGBA8,

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

    struct Texture2D {
        Texture2D() = default;
        ~Texture2D();

        static Ref<Texture2D> Create(u32 width, u32 height);
        static Ref<Texture2D> Create(const FS::Path& path, TextureFormat desiredFormat = TextureFormat::RGBA8);
        static Ref<Texture2D> Create(Ref<Image> image);
        static Ref<Texture2D> Create(void* pixels, u32 width, u32 height, TextureFormat pixelFormat, TextureFiltering filter = TextureFiltering::Linear);

        void Delete();
    
        void* ReadPixels();
    
        u32 ID = 0;
        u64 BindlessHandle = 0; // for bindless textures
        u32 Width = 0;
        u32 Height = 0;
        TextureFormat Format = TextureFormat::RGBA8;
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
    
        u32 ID = 0;
        FramebufferSpecification Specification;
        std::vector<Ref<Texture2D>> Attachments;

    private:
        void Invalidate();
    };

} // namespace Blackberry