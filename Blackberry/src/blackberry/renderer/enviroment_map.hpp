#pragma once

#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    struct EnviromentMap {
        static Ref<EnviromentMap> Create(const FS::Path& hdri);

        Ref<Texture2D> Skybox;
        Ref<Texture2D> Irradiance;
    };

} // namespace Blackberry