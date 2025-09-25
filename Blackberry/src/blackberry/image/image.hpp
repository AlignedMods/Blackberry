#pragma once

#include "blackberry/types.hpp"

#include <filesystem>

namespace Blackberry {

    class Image {
    public:
        Image();
        Image(const std::filesystem::path& path);
        Image(void* data, u32 width, u32 height);
        ~Image();

        void LoadFromPath(const std::filesystem::path& path);

        i32 GetWidth() const;
        i32 GetHeight() const;
        void* GetData() const;

    private:
        void* m_Data;
        i32 m_Width, m_Height;
        i32 m_Channels;
    };

} // namespace Blackberry