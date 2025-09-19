#pragma once

#include "types.hpp"

#include <filesystem>

namespace Blackberry {

    Image LoadImageFromFile(const std::filesystem::path& path);
    void UnloadImage(const Image& image);

} // namespace Blackberry