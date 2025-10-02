#pragma once

#include "blackberry/types.hpp"

#define BL_SYMBOLIC_COMPONENT(name) using name = u8

namespace Blackberry {

    BL_SYMBOLIC_COMPONENT(Drawable);

    struct Transform {
        BlVec2 Position;
        BlVec2 Dimensions;
    };

    struct Color {
        BlColor Fill;
    };

} // namespace Blackberry