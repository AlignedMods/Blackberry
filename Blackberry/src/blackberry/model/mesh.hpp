#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"

#include <filesystem>

namespace Blackberry {

    // NOTE: a mesh in Blackberry is what a lot of engines would call a "model"
    class Mesh {
    public:
        static Mesh Create(const std::filesystem::path& path);

    public:
        // data about mesh
        std::vector<BlVec3<f32>> Positions;
        std::vector<BlVec3<f32>> Normals;
        std::vector<BlVec4<f32>> Colors;
        std::vector<BlVec2<f32>> TexCoords;
        std::vector<u32> Indices;

        Texture2D Texture;
    };

} // namespace Blackberry