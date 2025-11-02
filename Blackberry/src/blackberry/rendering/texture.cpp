#include "blackberry/rendering/texture.hpp"
#include "blackberry/core/util.hpp"

#include "glad/glad.h"

BlTexture::BlTexture() {}

void BlTexture::Create(u32 width, u32 height) {
    Width = width;
    Height = height;
    Format = Blackberry::ImageFormat::RGBA8;

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void BlTexture::Create(const std::filesystem::path& path) {
    Blackberry::Image image(path);
    if (image.GetData() == nullptr) {
        BL_ERROR("Failed to load texture from path: {}", path.string());
        return;
    }

    Create(image);
}

void BlTexture::Create(const Blackberry::Image& image) {
    Create(image.GetData(), image.GetWidth(), image.GetHeight(), image.GetFormat());
}

void BlTexture::Create(void* pixels, u32 width, u32 height, Blackberry::ImageFormat pixelFormat) {
    Width = width;
    Height = height;
    Format = pixelFormat;

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // <<< very important!

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint format = GL_RGBA;
    GLuint glFormat = GL_RGBA8;

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
    }

    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, Width, Height, 0, format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void BlTexture::Delete() {
    glDeleteTextures(1, &ID);
    ID = 0;
    Width = 0;
    Height = 0;
}

void* BlTexture::ReadPixels() {
    u8* pixels = new u8[Width * Height * 4];

    glBindTexture(GL_TEXTURE_2D, ID);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    return pixels;
}

BlRenderTexture::BlRenderTexture() {}

void BlRenderTexture::Create(u32 width, u32 height) {
    if (width == 0 || height == 0) { return; }
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);

    BlTexture tex;
    tex.Create(width, height);
    Texture = tex;

    glBindTexture(GL_TEXTURE_2D, Texture.ID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture.ID, 0);

    glGenTextures(1, &DepthTexture.ID);
    glBindTexture(GL_TEXTURE_2D, DepthTexture.ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture.ID, 0);

    BL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Render Texture not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlRenderTexture::Resize(u32 width, u32 height) {
    Delete();
    Create(width, height);
}

void BlRenderTexture::Delete() {
    Texture.Delete();
    glDeleteFramebuffers(1, &ID);
}