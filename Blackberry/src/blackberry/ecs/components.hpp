#pragma once

#include "blackberry/core/types.hpp"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <filesystem>
#include <string>

namespace Blackberry {

    class Font;

    enum class MeshType : u16 {
        Plane = 0,
        Cube = 1,
        Sphere = 2,
        Cylinder = 3
    };

    enum class RigidBodyType : u16 {
        Static = 0,
        Dynamic = 1,
        Kinematic = 2
    };

    struct TagComponent {
        std::string Name;
        u64 UUID = 0;
    };

    struct TransformComponent {
        BlVec3<f32> Position;
        BlVec3<f32> Rotation; // in degrees
        BlVec3<f32> Scale;

        inline glm::mat4 GetMatrix() const {
            glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(Position.x, Position.y, Position.z));
            glm::mat4 rot = glm::toMat4(glm::quat(glm::vec3(glm::radians(Rotation.x), glm::radians(Rotation.y), glm::radians(Rotation.z))));
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(Scale.x, Scale.y, Scale.z));

            return pos * rot * scale;
        }
    };

    struct MeshRendererComponent {
        BlColor Color = BlColor(0xff, 0xff, 0xff, 0xff);
        u64 MeshHandle = 0;
    };

    struct CameraComponent {
        f32 FOV = 60.0f;
        f32 Near = 0.1f;
        f32 Far = 10.0f;
        f32 Zoom = 1.0f;

        bool Active = true;
    };

    struct RigidBodyComponent {
        RigidBodyType Type = RigidBodyType::Static;

        // NOTE: Do NOT serialize/allow editing of these fields directly
        // they are only here so they can be accessed through the physics system/displayed as read only values in debug menus
        // also i won't make them private since then i would have to add getters or add "friend class" which is stupid
        BlVec3<f32> Velocity;
        BlVec3<f32> Acceleration;
        BlVec3<f32> Force;

        f32 Mass = 10.0f;

        inline void AddForce(BlVec3<f32> force) {
            Force += force;
        }
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