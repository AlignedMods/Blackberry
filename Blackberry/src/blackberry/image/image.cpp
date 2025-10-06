#include "blackberry/image/image.hpp"
#include "blackberry/log.hpp"
#include "blackberry/application/renderer.hpp"
#include "blackberry/application/application.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace Blackberry {

    Image::Image()
        : m_Data(nullptr), m_Width(0), m_Height(0), m_Channels(0) {}

    Image::Image(const std::filesystem::path& path) {
        LoadFromPath(path);
    }

    Image::Image(void* data, u32 width, u32 height, ImageFormat format)
        : m_Data(data), m_Width(width), m_Height(height), m_Format(format) {}

    Image::~Image() {
        BL_INFO("Destructing Image!");
        stbi_image_free(static_cast<void*>(m_Data));

        m_Data = nullptr;
    }

    void Image::LoadFromPath(const std::filesystem::path& path) {
        m_Data = stbi_load(path.string().c_str(), &m_Width, &m_Height, &m_Channels, 4);
        m_Format = ImageFormat::RGBA8;

        if (!m_Data) {
            BL_ERROR("Failed to load image %s!", path.string().c_str());
        }
    }

    void Image::LoadFromMemory(void* data, u32 width, u32 height, ImageFormat format) {
        m_Width = width;
        m_Height = height;
        m_Data = data;
        m_Format = format;
    }

    void Image::LoadFromTexture(BlTexture texture) {
        Renderer& renderer = Application::Get().GetRenderer();

        m_Data = renderer.GetTextureData(texture);
        m_Width = static_cast<i32>(renderer.GetTexDims(texture).x);
        m_Height = static_cast<i32>(renderer.GetTexDims(texture).y);
        m_Format = ImageFormat::RGBA8;
    }

    void Image::WriteOut(const std::string& fileName) {
        stbi_write_png(fileName.c_str(), m_Width, m_Height, 1, m_Data, m_Width);
    }

    i32 Image::GetWidth() const {
        return m_Width;
    }

    i32 Image::GetHeight() const {
        return m_Height;
    }

    void* Image::GetData() const {
        return m_Data;
    }

    ImageFormat Image::GetFormat() const {
        return m_Format;
    }

} // namespace Blackberry
