#pragma once

#include "blackberry/core/types.hpp"

#include <filesystem>

namespace Blackberry {

    // NOTE: a mesh in Blackberry is what a lot of engines would call a "model"
    class Mesh {
    public:
        static Mesh Create(const std::filesystem::path& path);

    public:
        // data about mesh
        std::vector<BlVec3<f32>> Positions;
        std::vector<u32> Indices;
    };

} // namespace Blackberry