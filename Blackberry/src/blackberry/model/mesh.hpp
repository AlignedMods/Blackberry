#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/renderer/texture.hpp"
#include "blackberry/model/material.hpp"

namespace Blackberry {

    // Basic mesh struct which holds info about a mesh (meshes can be created through models)
    struct Mesh {
        BlMat4 Transform = BlMat4(1.0f);

        // data about mesh
        std::vector<BlVec3> Positions;
        std::vector<BlVec3> Normals;
        std::vector<BlVec4> Colors;
        std::vector<BlVec2> TexCoords;
        std::vector<u32> Indices;

        // NOTE: This index should NEVER be invalid, if there are no materials in a model a default one will be always be created!
        u32 MaterialIndex = 0;
    };

} // namespace Blackberry