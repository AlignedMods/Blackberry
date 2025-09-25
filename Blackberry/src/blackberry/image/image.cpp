#include "blackberry/image/image.hpp"
#include "blackberry/log.hpp"

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

    Image::Image(void* data, u32 width, u32 height)
        : m_Data(data), m_Width(width), m_Height(height) {}

    Image::~Image() {
        Log(Log_Info, "Destructing Image!");
        stbi_image_free(static_cast<void*>(m_Data));
    }

    void Image::LoadFromPath(const std::filesystem::path& path) {
        m_Data = stbi_load(path.string().c_str(), &m_Width, &m_Height, &m_Channels, 4);

        if (!m_Data) {
            Log(Log_Error, "Failed to load image %s!", path.string().c_str());
        }
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

} // namespace Blackberry
