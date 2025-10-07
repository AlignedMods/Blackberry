#pragma once

#include "blackberry/types.hpp"
#include "blackberry/font/font.hpp"

#define BL_SYMBOLIC_COMPONENT(name) using name = u8

namespace Blackberry::Components {

    struct Tag {
        std::string Name;
    };

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

    struct Text {
        Blackberry::Font* Font;
        u32 FontSize;
        std::string Contents;
    };

} // namespace Blackberry::Components