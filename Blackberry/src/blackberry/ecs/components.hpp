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
        BlTexture Texture;
        BlRec Area;
    };

    struct Drawable {
        BlColor Color = BlColor(0xff, 0xff, 0xff, 0xff);
    };

    struct Text {
        Blackberry::Font* Font;
        u32 FontSize = 24;
        std::string Contents;
    };

} // namespace Blackberry::Components