#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"
#include "blackberry/assets/asset_manager.hpp"

#define BL_SYMBOLIC_COMPONENT(name) using name = u8

namespace Blackberry::Components {

    struct Tag {
        std::string Name;
        u64 UUID;
    };

    struct Transform {
        BlVec2 Position;
        BlVec2 Dimensions;
    };

    struct Material {
        BlTexture Texture;
        BlRec Area = BlRec(0, 0, Texture.Width, Texture.Height); // (aligned) NOTE: very sketchy, this depends on texture being initialized first!
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