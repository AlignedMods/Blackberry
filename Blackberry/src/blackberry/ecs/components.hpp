#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/font/font.hpp"
#include "blackberry/assets/asset_manager.hpp"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Blackberry {

    enum class ShapeType : u16 {
        Triangle = 0,
        Rectangle = 1,
        Circle = 2,
        Polygon = 3
    };

    struct TagComponent {
        std::string Name;
        u64 UUID = 0;
    };

    struct TransformComponent {
        BlVec3 Position;
        f32 Rotation = 0.0f; // in degrees
        BlVec2 Dimensions;
    };

    struct ShapeRendererComponent {
        BlColor Color = BlColor(0xff, 0xff, 0xff, 0xff);
        ShapeType Shape = ShapeType::Rectangle;
    };

    struct SpriteRendererComponent {
        BlColor Color = BlColor(0xff, 0xff, 0xff, 0xff);
        ShapeType Shape = ShapeType::Rectangle;

        u64 TextureHandle = 0;
        BlRec Area;
    };

    struct VelocityComponent {
        BlVec2 Acceleration;
    };

    struct TextComponent {
        Blackberry::Font* Font;
        u32 FontSize = 24;
        std::string Contents;
    };

    struct ScriptComponent {
        std::filesystem::path ModulePath;
        std::filesystem::path FilePath;
        bool IsLoaded = false;
    };

} // namespace Blackberry