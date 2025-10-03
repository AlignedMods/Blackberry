#pragma once

#include "blackberry/types.hpp"

#define BL_SYMBOLIC_COMPONENT(name) using name = u8

namespace Blackberry::Components {

    struct Transform {
        BlVec2 Position;
        BlVec2 Dimensions;
    };

    struct Material {
        BlTexture Texture = nullptr;
        BlRec Area;
    };

    struct Drawable {
        BlColor Color;
    };

} // namespace Blackberry::Components