#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"
#include "blackberry/assets/asset_manager.hpp"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define BL_SYMBOLIC_COMPONENT(name) using name = u8

namespace Blackberry::Components {

    enum class Shape : u16 {
        Triangle = 0,
        Rectangle = 1,
        Circle = 2,
        Polygon = 3
    };

    struct Tag {
        std::string Name;
        u64 UUID;
    };

    struct Transform {
        BlVec3 Position;
        f32 Rotation = 0.0f; // in degrees
        BlVec2 Dimensions;
    };

    struct Material {
        u64 TextureHandle = 0;
        BlRec Area;
    };

    struct Drawable {
        BlColor Color = BlColor(0xff, 0xff, 0xff, 0xff);
        Shape ShapeType = Shape::Rectangle;
    };

    struct Velocity {
        BlVec2 Acceleration;
    };

    struct Text {
        Blackberry::Font* Font;
        u32 FontSize = 24;
        std::string Contents;
    };

    struct Script {
        std::filesystem::path ModulePath;
        std::filesystem::path FilePath;
        bool IsLoaded = false;
    };

} // namespace Blackberry::Components