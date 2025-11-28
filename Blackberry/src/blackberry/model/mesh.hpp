#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    // Basic mesh struct which holds info about a mesh (meshes can be created through models)
    struct Mesh {
        // data about mesh
        std::vector<BlVec3<f32>> Positions;
        std::vector<BlVec3<f32>> Normals;
        std::vector<BlVec4<f32>> Colors;
        std::vector<BlVec2<f32>> TexCoords;
        std::vector<u32> Indices;

        // TODO (aligned): Replace with materials once we get them
        Texture2D Texture;
    };

} // namespace Blackberry