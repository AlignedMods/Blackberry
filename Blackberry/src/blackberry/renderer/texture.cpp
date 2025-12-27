#include "blackberry/renderer/texture.hpp"
#include "blackberry/core/util.hpp"

#include "glad/gl.h"
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Blackberry {

    Texture2D::~Texture2D() {
        Delete();
    }

    Ref<Texture2D> Texture2D::Create(u32 width, u32 height) {
        Ref<Texture2D> tex = CreateRef<Texture2D>();

        tex->Width= width;
        tex->Height = height;
        tex->Format = Blackberry::TextureFormat::RGBA8;
    
        glGenTextures(1, &tex->ID);
        glBindTexture(GL_TEXTURE_2D, tex->ID);
    
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
    
    Ref<Texture2D> Texture2D::Create(const FS::Path& path, TextureFormat desiredFormat) {
        int width = 0, height = 0;

        bool useFloatingPoint = false;
        int channelCount = 0;

        switch (desiredFormat) {
            case TextureFormat::RGB8: useFloatingPoint = false; channelCount = 3; break;
            case TextureFormat::RGBA8: useFloatingPoint = false; channelCount = 4; break;

            case TextureFormat::RGB16F: useFloatingPoint = true; channelCount = 3; break;
            case TextureFormat::RGBA16F: useFloatingPoint = true; channelCount = 4; break;
        }

        void* pixels = nullptr;

        std::string strPath = path.String();

        if (!useFloatingPoint) {
            pixels = stbi_load(strPath.c_str(), &width, &height, nullptr, channelCount);
        } else {
            pixels = stbi_loadf(strPath.c_str(), &width, &height, nullptr, channelCount);
        }

        if (pixels == nullptr) {
            BL_ERROR("Failed to load texture from path: {}", path.String());
            return CreateRef<Texture2D>();
        }
    
        Ref<Texture2D> texture = Create(pixels, width, height, desiredFormat);

        stbi_image_free(pixels);

        return texture;
    }

    Ref<Texture2D> Texture2D::Create(Ref<Image> image) {
        TextureFormat format = TextureFormat::RGB8;

        switch (image->Format) {
            case ImageFormat::U8: format = TextureFormat::U8; break;
            case ImageFormat::RGB8: format = TextureFormat::RGB8; break;
            case ImageFormat::RGBA8: format = TextureFormat::RGBA8; break;
            case ImageFormat::RGB32F: format = TextureFormat::RGB16F; break;
            case ImageFormat::RGBA32F: format = TextureFormat::RGBA16F; break;
            default: BL_CORE_WARN("Unknown ImageFormat!"); break;
        }

        Ref<Texture2D> tex = Create(image->Pixels, image->Width, image->Height, format);

        return tex;
    }
    
    Ref<Texture2D> Texture2D::Create(void* pixels, u32 width, u32 height, TextureFormat pixelFormat, TextureFiltering filter) {
        Ref<Texture2D> tex = CreateRef<Texture2D>();

        tex->Width = width;
        tex->Height = height;
        tex->Format = pixelFormat;
    
        GLuint id = 0;
        glCreateTextures(GL_TEXTURE_2D, 1, &id);

        BL_CORE_INFO("OpenGL context: {}, Texture ID: {}", reinterpret_cast<void*>(glfwGetCurrentContext()), id);

        tex->ID = id;
    
        glTextureParameteri(tex->ID, GL_TEXTURE_WRAP_S, GL_REPEAT);	
        glTextureParameteri(tex->ID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(tex->ID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        if (filter == TextureFiltering::Nearest) {
            glTextureParameteri(tex->ID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTextureParameteri(tex->ID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    
        GLuint format = GL_RGBA;
        GLuint glFormat = GL_RGBA8;
        GLuint type = GL_UNSIGNED_BYTE;
    
        switch (pixelFormat) {
            case TextureFormat::U8:
                format = GL_RED;
                glFormat = GL_R8;
                break;
            case TextureFormat::RGB8:
                format = GL_RGB;
                glFormat = GL_RGB8;
                break;
            case TextureFormat::RGBA8:
                format = GL_RGBA;
                glFormat = GL_RGBA8;
                break;
            case TextureFormat::RGB16F:
                format = GL_RGB;
                glFormat = GL_RGB16F;
                type = GL_FLOAT;
                break;
            case TextureFormat::RGBA16F:
                format = GL_RGBA;
                glFormat = GL_RGBA16F;
                type = GL_FLOAT;
                break;
        }
    
        glTextureStorage2D(tex->ID, 1, glFormat, width, height);
        glTextureSubImage2D(tex->ID, 0, 0, 0, width, height, format, type, pixels);

        glGenerateTextureMipmap(tex->ID);

        tex->BindlessHandle = glGetTextureHandleARB(tex->ID);
        if (tex->BindlessHandle == 0 || !glIsTexture(tex->ID)) {
            BL_CORE_CRITICAL("Failed to create bindless texture!");
            exit(1);
        }
        glMakeTextureHandleResidentARB(tex->BindlessHandle);

        return tex;
    }
    
    void Texture2D::Delete() {
        BL_CORE_INFO("Destroying texture!");

        glDeleteTextures(1, &ID);
        glMakeTextureHandleNonResidentARB(BindlessHandle);
        ID = 0;
        BindlessHandle = 0;
        Width = 0;
        Height = 0;
    }
    
    void* Texture2D::ReadPixels() {
        u8* pixels = new u8[Width * Height * 4];
    
        glBindTexture(GL_TEXTURE_2D, ID);
    
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    
        glBindTexture(GL_TEXTURE_2D, 0);
    
        return pixels;
    }
    
    Framebuffer::~Framebuffer() {
        Delete();
    }

    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec) {
        Ref<Framebuffer> tex = CreateRef<Framebuffer>();
        tex->Specification = spec;
        tex->Invalidate();

        return tex;
    }
    
    void Framebuffer::Delete() {
        glDeleteFramebuffers(1, &ID);
        Attachments.clear();
        ID = 0;
    }

    void Framebuffer::Resize(u32 width, u32 height) {
        Delete();
        Specification.Width = width;
        Specification.Height = height;
        Invalidate();
    }

    void Framebuffer::ClearAttachmentInt(u32 attachment, int value) {
        BL_ASSERT(Specification.Attachments.at(attachment).Type == FramebufferAttachmentType::ColorR32I, "Not an integer attachment!");

        glClearTexImage(Attachments.at(attachment)->ID, 0, GL_RED_INTEGER, GL_INT, &value);
    }

    void Framebuffer::ClearAttachmentFloat(u32 attachment, f32 value) {
        BL_ASSERT(Specification.Attachments.at(attachment).Type == FramebufferAttachmentType::ColorR32F, "Not a floating point attachment!");

        glClearTexImage(Attachments.at(attachment)->ID, 0, GL_RED, GL_FLOAT, &value);
    }

    void* Framebuffer::ReadPixels(u32 attachment, BlVec2 position, BlVec2 dimensions, u32 sizeBytes) {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

        GLenum format = 0;
        GLenum type = 0;

        switch (Specification.Attachments.at(attachment).Type) {
            case FramebufferAttachmentType::ColorRGBA16F: format = GL_RGBA; type = GL_FLOAT; break;
            case FramebufferAttachmentType::ColorR32I: format = GL_RED_INTEGER; type = GL_UNSIGNED_BYTE; break;
            case FramebufferAttachmentType::ColorR32F: format = GL_RED; type = GL_FLOAT; break;
        }

        void* pixels = malloc(dimensions.x * dimensions.y * sizeBytes);

        glReadPixels(position.x, position.y, dimensions.x, dimensions.y, format, type, pixels);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return pixels;
    }

    void Framebuffer::BlitDepthBuffer(Ref<Framebuffer> other) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, ID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, other->ID);
        glBlitFramebuffer(
          0, 0, Specification.Width, Specification.Height, 0, 0, Specification.Width, Specification.Height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // glBlitNamedFramebuffer(ID, other->ID, 
        //                        0, 0, Specification.Width, Specification.Height, 
        //                        0, 0, other->Specification.Width, other->Specification.Height, 
        //                        GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    }

    void Framebuffer::Invalidate() {
        if (Specification.Width == 0 || Specification.Height == 0) return;

        glCreateFramebuffers(1, &ID);
    
        for (auto& attachment : Specification.Attachments) {
            Ref<Texture2D> texAttachment = CreateRef<Texture2D>();
            texAttachment->Width = Specification.Width;
            texAttachment->Height = Specification.Height;
            bool createBindlessHandle = true;

            if (attachment.Type == FramebufferAttachmentType::ColorR8) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_R8, Specification.Width, Specification.Height);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Width, Specification.Height, GL_RED, GL_UNSIGNED_BYTE, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0 + attachment.Attachment, id, 0);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = true;
            } else if (attachment.Type == FramebufferAttachmentType::ColorRGBA8) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_RGBA8, Specification.Width, Specification.Height);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Width, Specification.Height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0 + attachment.Attachment, id, 0);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = true;
            } else if (attachment.Type == FramebufferAttachmentType::ColorRGBA16F) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_RGBA16F, Specification.Width, Specification.Height);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Width, Specification.Height, GL_RGBA, GL_FLOAT, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0 + attachment.Attachment, id, 0);

                texAttachment->Format = TextureFormat::RGBA16F;
                texAttachment->ID = id;

                createBindlessHandle = true;
            } else if (attachment.Type == FramebufferAttachmentType::Depth) {
                u32 id = 0;

                glCreateRenderbuffers(1, &id);
                glNamedRenderbufferStorage(id, GL_DEPTH_COMPONENT24, Specification.Width, Specification.Height);
                glNamedFramebufferRenderbuffer(ID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = false;
            } else if (attachment.Type == FramebufferAttachmentType::Depth24) {
                u32 id = 0;

                glCreateRenderbuffers(1, &id);
                glNamedRenderbufferStorage(id, GL_DEPTH_COMPONENT24, Specification.Width, Specification.Height);
                glNamedFramebufferRenderbuffer(ID, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = false;
            } else if (attachment.Type == FramebufferAttachmentType::ColorR32I) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_R32I, Specification.Width, Specification.Height);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Width, Specification.Height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0 + attachment.Attachment, id, 0);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = true;
            } else if (attachment.Type == FramebufferAttachmentType::ColorR32F) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_R32F, Specification.Width, Specification.Height);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Width, Specification.Height, GL_RED, GL_FLOAT, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glNamedFramebufferTexture(ID, GL_COLOR_ATTACHMENT0 + attachment.Attachment, id, 0);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = true;
            }

            if (createBindlessHandle) {
                texAttachment->BindlessHandle = glGetTextureHandleARB(texAttachment->ID);
                if (texAttachment->BindlessHandle == 0 || !glIsTexture(texAttachment->ID)) {
                    BL_CORE_CRITICAL("Failed to create bindless texture!");
                    exit(1);
                }
                glMakeTextureHandleResidentARB(texAttachment->BindlessHandle);
            }

            Attachments.push_back(texAttachment);
        }

        // Tell OpenGL which attachments we are using for rendering
        std::vector<u32> activeAttachments;

        for (auto& attachment : Specification.ActiveAttachments) {
            activeAttachments.push_back(GL_COLOR_ATTACHMENT0 + attachment);
        }
        glNamedFramebufferDrawBuffers(ID, activeAttachments.size(), activeAttachments.data());

        BL_ASSERT(glCheckNamedFramebufferStatus(ID, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete!");
    }

} // namespace Blackberry