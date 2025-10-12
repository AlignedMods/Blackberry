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

void BlTexture::Create(const Blackberry::Image& image) {
    Width = image.GetWidth();
    Height = image.GetHeight();
    Format = image.GetFormat();

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // <<< very important!

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint format = GL_RGBA;
    GLuint glFormat = GL_RGBA;

    switch (image.GetFormat()) {
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

    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, Width, Height, 0, format, GL_UNSIGNED_BYTE, image.GetData());
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void BlTexture::Delete() {
    glDeleteTextures(1, &ID);
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

    BL_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Render Texture not complete!");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BlRenderTexture::Delete() {
    Texture.Delete();
    glDeleteFramebuffers(1, &ID);
}