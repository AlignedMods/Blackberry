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

        tex->Size.x = width;
        tex->Size.y = height;
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

        tex->Size.x = width;
        tex->Size.y = height;
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
    
    RenderTexture::~RenderTexture() {
        Delete();
    }

    Ref<RenderTexture> RenderTexture::Create(const RenderTextureSpecification& spec) {
        Ref<RenderTexture> tex = CreateRef<RenderTexture>();
        tex->Specification = spec;
        tex->Invalidate();

        return tex;
    }
    
    void RenderTexture::Delete() {
        glDeleteFramebuffers(1, &ID);
        Attachments.clear();
        ID = 0;
    }

    void RenderTexture::Resize(BlVec2<u32> size) {
        Delete();
        Specification.Size = size;
        Invalidate();
    }

    void RenderTexture::Invalidate() {
        if (Specification.Size.x == 0 || Specification.Size.y == 0) return;

        glGenFramebuffers(1, &ID);
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    
        for (auto& attachment : Specification.Attachments) {
            Ref<Texture2D> texAttachment = CreateRef<Texture2D>();
            texAttachment->Size = Specification.Size;
            bool createBindlessHandle = true;

            if (attachment.Type == RenderTextureAttachmentType::ColorRGBA8) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_RGBA8, Specification.Size.x, Specification.Size.y);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Size.x, Specification.Size.y, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment.Attachment, GL_TEXTURE_2D, id, 0);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = true;
            } else if (attachment.Type == RenderTextureAttachmentType::ColorRGBA16F) {
                u32 id = 0;

                glCreateTextures(GL_TEXTURE_2D, 1, &id);

                glTextureStorage2D(id, 1, GL_RGBA16F, Specification.Size.x, Specification.Size.y);
                glTextureSubImage2D(id, 0, 0, 0, Specification.Size.x, Specification.Size.y, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

                glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment.Attachment, GL_TEXTURE_2D, id, 0);

                texAttachment->Format = TextureFormat::RGBA16F;
                texAttachment->ID = id;

                createBindlessHandle = true;
            } else if (attachment.Type == RenderTextureAttachmentType::Depth) {
                u32 id = 0;

                glGenRenderbuffers(1, &id);
                glBindRenderbuffer(GL_RENDERBUFFER, id);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Specification.Size.x, Specification.Size.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = false;
            } else if (attachment.Type == RenderTextureAttachmentType::Depth24) {
                u32 id = 0;

                glGenRenderbuffers(1, &id);
                glBindRenderbuffer(GL_RENDERBUFFER, id);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, Specification.Size.x, Specification.Size.y);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id);

                texAttachment->Format = TextureFormat::RGBA8;
                texAttachment->ID = id;

                createBindlessHandle = false;
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
        glDrawBuffers(activeAttachments.size(), activeAttachments.data());

        BL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Render Texture not complete!");
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

} // namespace Blackberry