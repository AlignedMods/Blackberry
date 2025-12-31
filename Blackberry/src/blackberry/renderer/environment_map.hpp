#pragma once

#include "blackberry/renderer/texture.hpp"

namespace Blackberry {

    struct EnvironmentMap {
        static Ref<EnvironmentMap> Create(const FS::Path& hdri);

        Ref<Texture> Skybox;
        Ref<Texture> Prefilter;
        Ref<Texture> Irradiance;
        Ref<Texture> BrdfLUT;
    };

} // namespace Blackberry