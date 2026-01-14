#pragma once

#include "blackberry/model/mesh.hpp"

namespace Blackberry {

    class Model {
    public:
        static Model Create(const FS::Path& path);
        static Model Create(const std::vector<Mesh>& meshes);

    public:
        std::vector<Mesh> Meshes;
        std::vector<Material> Materials; // This ALWAYS has at LEAST one material! (default gets created if none are contained!)
    };

} // namespace Blackberry