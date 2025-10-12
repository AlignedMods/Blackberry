#pragma once

#include "blackberry/core/types.hpp"
// #include "blackberry/rendering/texture.hpp"

#include <filesystem>

namespace Blackberry {

    enum class ImageFormat {
        U8,
        RGB8,
        RGBA8
    };

    class Image {
    public:
        Image();
        Image(const std::filesystem::path& path);
        Image(const void* data, u32 width, u32 height, ImageFormat format);
        ~Image();

        void LoadFromPath(const std::filesystem::path& path);
        void LoadFromMemory(void* data, u32 width, u32 height, ImageFormat format);
        // void LoadFromTexture(BlTexture texture);

        void WriteOut(const std::string& fileName);

        i32 GetWidth() const;
        i32 GetHeight() const;
        void* GetData() const;
        ImageFormat GetFormat() const;

    private:
        void* m_Data;
        i32 m_Width, m_Height;
        i32 m_Channels;
        ImageFormat m_Format;
    };

} // namespace Blackberry