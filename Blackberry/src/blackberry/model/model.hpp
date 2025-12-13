#pragma once

#include "blackberry/model/mesh.hpp"

namespace Blackberry {

    class Model {
    public:
        static Model Create(const FS::Path& path);
        static Model Create(const std::vector<Mesh>& meshes);

    public:
        std::vector<Mesh> Meshes;
        u32 MeshCount = 0;
    };

} // namespace Blackberry