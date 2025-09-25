#pragma once

#include "blackberry/image/image.hpp"

#include <filesystem>

namespace Blackberry {

    class Font {
    public:
        Font();
        Font(const std::filesystem::path& path);
        ~Font();

        Image LoadFontFromFile(const std::filesystem::path& path);
    };

} // namespace Blackberry