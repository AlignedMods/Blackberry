#pragma once

#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    struct EnviromentMap {
        static Ref<EnviromentMap> Create(const FS::Path& hdri);

        Ref<Texture> Skybox;
        Ref<Texture> Prefilter;
        Ref<Texture> Irradiance;
        Ref<Texture> BrdfLUT;
    };

} // namespace Blackberry