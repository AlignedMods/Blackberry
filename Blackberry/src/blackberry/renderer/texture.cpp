#include "blackberry/renderer/texture.hpp"
#include "blackberry/core/util.hpp"

#include "glad/glad.h"

namespace Blackberry {

    Texture2D Texture2D::Create(u32 width, u32 height) {
        Texture2D tex;

        tex.Size.x = width;
        tex.Size.y = height;
        tex.Format = Blackberry::ImageFormat::RGBA8;
    
        glGenTextures(1, &tex.ID);
        glBindTexture(GL_TEXTURE_2D, tex.ID);
    
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // glGenerateMipmap(GL_TEXTURE_2D);
    
        glBindTexture(GL_TEXTURE_2D, 0);

        return tex;
    }
    
    Texture2D Texture2D::Create(const std::filesystem::path& path) {
        Blackberry::Image image = Blackberry::Image::Create(path);
        if (image.Data == nullptr) {
            BL_ERROR("Failed to load texture from path: {}", path.string());
            return Texture2D{};
        }
    
        return Create(image);
    }
    
    Texture2D Texture2D::Create(const Blackberry::Image& image) {
        return Create(image.Data, image.Width, image.Height, image.Format);
    }
    
    Texture2D Texture2D::Create(void* pixels, u32 width, u32 height, Blackberry::ImageFormat pixelFormat, TextureFiltering filter) {
        Texture2D tex;

        tex.Size.x = width;
        tex.Size.y = height;
        tex.Format = pixelFormat;
    
        glGenTextures(1, &tex.ID);
        glBindTexture(GL_TEXTURE_2D, tex.ID);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // <<< very important!
    
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        if (filter == TextureFiltering::Nearest) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    
        GLuint format = GL_RGBA;
        GLuint glFormat = GL_RGBA8;
        GLuint type = GL_UNSIGNED_BYTE;
    
        switch (pixelFormat) {
            case Blackberry::ImageFormat::U8:
                format = GL_RED;
                glFormat = GL_R8;
                break;
            case Blackberry::ImageFormat::RGB8:
                format = GL_RGB;
                glFormat = GL_RGB8;
                break;
            case Blackberry::ImageFormat::RGBA8:
                format = GL_RGBA;
                glFormat = GL_RGBA8;
                break;
            case Blackberry::ImageFormat::F32:
                format = GL_RED;
                glFormat = GL_R32F;
                type = GL_FLOAT;
                break;
            case Blackberry::ImageFormat::RGB32F:
                format = GL_RGB;
                glFormat = GL_RGB32F;
                type = GL_FLOAT;
                break;
        }
    
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, tex.Size.x, tex.Size.y, 0, format, type, pixels);
        glGenerateMipmap(GL_TEXTURE_2D);
    
        glBindTexture(GL_TEXTURE_2D, 0);

        return tex;
    }
    
    void Texture2D::Delete() {
        glDeleteTextures(1, &ID);
        ID = 0;
        Size.x = 0;
        Size.y = 0;
    }
    
    void* Texture2D::ReadPixels() {
        u8* pixels = new u8[Size.x * Size.y * 4];
    
        glBindTexture(GL_TEXTURE_2D, ID);
    
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
        glBindTexture(GL_TEXTURE_2D, 0);
    
        return pixels;
    }
    
    RenderTexture RenderTexture::Create(const RenderTextureSpecification& spec) {
        RenderTexture tex;
        tex.Specification = spec;

        if (spec.Size.x == 0 || spec.Size.y == 0) return {};

        glGenFramebuffers(1, &tex.ID);
        glBindFramebuffer(GL_FRAMEBUFFER, tex.ID);
    
        for (auto& attachment : spec.Attachments) {
            Texture2D texAttachment;
            texAttachment.Size = spec.Size;

            if (attachment.Type == RenderTextureAttachmentType::ColorRGBA8) {
                u32 id = 0;

                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, spec.Size.x, spec.Size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment.Attachment, GL_TEXTURE_2D, id, 0);

                texAttachment.Format = ImageFormat::RGBA8;
                texAttachment.ID = id;
            } else if (attachment.Type == RenderTextureAttachmentType::ColorRGBA16F) {
                u32 id = 0;

                glGenTextures(1, &id);
                glBindTexture(GL_TEXTURE_2D, id);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, spec.Size.x, spec.Size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment.Attachment, GL_TEXTURE_2D, id, 0);

                texAttachment.Format = ImageFormat::RGBA32F;
                texAttachment.ID = id;
            } else if (attachment.Type == RenderTextureAttachmentType::Depth) {
                u32 id = 0;

                glGenRenderbuffers(1, &id);
                glBindRenderbuffer(GL_RENDERBUFFER, id);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, spec.Size.x, spec.Size.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);

                texAttachment.Format = ImageFormat::RGBA8;
                texAttachment.ID = id;
            }

            tex.Attachments.push_back(texAttachment);
        }

        // Tell OpenGL which attachments we are using for rendering
        std::vector<u32> activeAttachments;

        for (auto& attachment : spec.ActiveAttachments) {
            activeAttachments.push_back(GL_COLOR_ATTACHMENT0 + attachment);
        }
        glDrawBuffers(activeAttachments.size(), activeAttachments.data());

        BL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Render Texture not complete!");
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return tex;
    }
    
    void RenderTexture::Delete() {
        glDeleteFramebuffers(1, &ID);
    }

    void RenderTexture::Resize(BlVec2<u32> size) {
        Delete();
        Specification.Size = size;
        RenderTexture newTex = Create(Specification);

        Attachments = newTex.Attachments;
        ID = newTex.ID;
    }

} // namespace Blackberry