#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/scene/camera.hpp"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <filesystem>
#include <string>

namespace Blackberry {

    class Font;

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

    struct Transform3DComponent {
        BlVec3 Position;
        BlVec3 Rotation; // in degrees
        BlVec2 Dimensions;

        inline glm::mat4 GetMatrix() const {
            glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));
            glm::mat4 rot = glm::toMat4(glm::quat(glm::vec3(glm::radians(Rotation.x), glm::radians(Rotation.y), glm::radians(Rotation.z))));
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(Dimensions.x, Dimensions.y, 1.0f));

            return pos * rot * scale;
        }
    };

    struct Transform2DComponent {
        BlVec3 Position; // Vec3 cause of z position
        f32 Rotation = 0.0f; // in degrees
        BlVec2 Dimensions;

        inline glm::mat4 GetMatrix() const {
            glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(Dimensions.x, Dimensions.y, 1.0f));

            return pos * rot * scale;
        }
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

    struct CameraComponent {
        SceneCamera Camera;
        bool Active = true;
    };

    struct RigidBodyComponent {
        BlVec2 Velocity;
        BlVec2 Acceleration;
        BlVec2 Force;

        f32 Mass = 10.0f;
    };

    struct BoxColliderComponent {
        bool Active = true;
    };

    struct TextComponent {
        std::string Contents;
        u64 FontHandle = 0;

        f32 Kerning = 0.0f;
        f32 LineSpacing = 0.0f;
    };

    struct ScriptComponent {
        std::filesystem::path ModulePath;
        std::filesystem::path FilePath;
        bool IsLoaded = false;
    };

} // namespace Blackberry