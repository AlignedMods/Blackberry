#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/path.hpp"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <map>

namespace Blackberry {

    class Font;

    enum class RigidBodyType : u16 {
        Static = 0,
        Dynamic = 1,
        Kinematic = 2
    };

    enum class ForceType {
        Linear,
        Angular
    };

    struct TagComponent {
        std::string Name;
        u64 UUID = 0;
    };

    struct RelationshipComponent {
        u64 Parent = 0;
        u64 FirstChild = 0;
        u64 NextSibling = 0;
        u64 PrevSibling = 0;
    };

    struct TransformComponent {
        BlVec3 Position = BlVec3(0.0f);
        BlQuat Rotation = BlQuat(1.0f, 0.0f, 0.0f, 0.0f);
        BlVec3 Scale = BlVec3(1.0f);

        inline glm::mat4 GetMatrix() const {
            glm::mat4 pos = glm::translate(glm::mat4(1.0f), Position);
            glm::mat4 rot = glm::toMat4(Rotation);
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), Scale);

            return pos * rot * scale;
        }
    };

    struct MeshComponent {
        u64 MeshHandle = 0;
        std::map<u32, u64> MaterialHandles;
    };

    struct CameraComponent {
        f32 FOV = 60.0f;
        f32 Near = 0.1f;
        f32 Far = 10000.0f;
        f32 Zoom = 1.0f;

        bool Active = true;
    };

    struct RigidBodyComponent {
        RigidBodyType Type = RigidBodyType::Static;
        f32 Resitution = 1.0f;
        f32 Friction = 1.0f;
    };

    struct BoxColliderComponent {
        BlVec3 Scale = BlVec3(1.0f);
    };

    struct SphereColliderComponent {
        f32 Radius = 1.0f;
    };

    struct TextComponent {
        std::string Contents;
        u64 FontHandle = 0;

        f32 Kerning = 0.0f;
        f32 LineSpacing = 0.0f;
    };

    struct ScriptComponent {
        FS::Path ModulePath;
        FS::Path FilePath;
        bool IsLoaded = false;
    };

    struct DirectionalLightComponent {
        BlVec3 Color = BlVec3(1.0f);
        f32 Intensity = 1.0f;
    };

    struct PointLightComponent {
        BlVec3 Color = BlVec3(1.0f);
        f32 Radius = 10.0f;
        f32 Intensity = 1.0f;
    };

    struct SpotLightComponent {
        BlVec3 Color = BlVec3(1.0f);
        f32 Cutoff = 25.0f;
        f32 Intensity = 1.0f;
    };

    struct EnvironmentComponent {
        u64 EnvironmentMap = 0;
        f32 LevelOfDetail = 0.0f;
        bool EnableBloom = true;
        f32 BloomThreshold = 3.0f;
    };

} // namespace Blackberry