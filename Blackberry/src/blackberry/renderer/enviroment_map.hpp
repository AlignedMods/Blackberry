#pragma once

#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    struct EnviromentMap {
        static Ref<EnviromentMap> Create(const FS::Path& hdri);

        Ref<Texture2D> Prefilter;
        Ref<Texture2D> Irradiance;
        Ref<Texture2D> BrdfLUT;
    };

} // namespace Blackberry