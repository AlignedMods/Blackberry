#include "blackberry/scene/scene_serializer.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "yaml-cpp/yaml.h"

namespace YAML {
    
    template <>
    struct convert<BlVec3<f32>> {
        static Node encode(const BlVec3<f32>& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, BlVec3<f32>& rhs) {
            if (!node.IsSequence() || node.size() != 3) {
                return false;
            }

            rhs.x = node[0].as<f32>();
            rhs.y = node[1].as<f32>();
            rhs.z = node[2].as<f32>();

            return true;
        }
    };

} // namespace YAML

namespace Blackberry {

    static const char* RigidBodyTypeToString(RigidBodyType type) {
        switch (type) {
            case RigidBodyType::Static: return "Static"; break;
            case RigidBodyType::Dynamic: return "Dynamic"; break;
            case RigidBodyType::Kinematic: return "Kinematic"; break;
        }

        BL_ASSERT(false, "Unreachable");
        return "";
    }

    static RigidBodyType StringToRigidBodyType(const std::string& str) {
        if (str == "Static") return RigidBodyType::Static;
        if (str == "Dynamic") return RigidBodyType::Dynamic;
        if (str == "Kinematic") return RigidBodyType::Kinematic;

        BL_ASSERT(false, "Unreachable");
        return RigidBodyType::Static;
    }

    static const char* ColliderTypeToString(ColliderType type) {
        switch (type) {
            case ColliderType::Cube: return "Cube"; break;
        }

        BL_ASSERT(false, "Unreachable");
        return "";
    }

    static ColliderType StringToColliderType(const std::string& str) {
        if (str == "Cube") return ColliderType::Cube;

        BL_ASSERT(false, "Unreachable");
        return ColliderType::Cube;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, BlVec3<f32> vec) {
        out << YAML::Flow << YAML::BeginSeq;
        out << vec.x << vec.y << vec.z;
        out << YAML::EndSeq;

        return out;
    }

    static void SerializeEntity(YAML::Emitter& out, Entity e) {
        BL_ASSERT(e.HasComponent<TagComponent>(), "All entities must have a TagComponent!");

        out << YAML::BeginMap; // Entity

        if (e.HasComponent<TagComponent>()) {
            auto& tag = e.GetComponent<TagComponent>();

            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            out << YAML::Key << "UUID" << YAML::Value << tag.UUID;
            out << YAML::Key << "Name" << YAML::Value << tag.Name;

            out << YAML::EndMap; // TagComponent
        }

        if (e.HasComponent<TransformComponent>()) {
            auto& transform = e.GetComponent<TransformComponent>();

            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            out << YAML::Key << "Position" << YAML::Value << transform.Position;
            out << YAML::Key << "Rotation" << YAML::Value << transform.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

            out << YAML::EndMap; // TransformComponent
        }

        if (e.HasComponent<MeshComponent>()) {
            auto& mesh = e.GetComponent<MeshComponent>();

            out << YAML::Key << "MeshComponent";
            out << YAML::BeginMap; // MeshComponent

            out << YAML::Key << "MeshHandle" << YAML::Value << mesh.MeshHandle;
            out << YAML::Key << "MaterialHandles" << YAML::Value << mesh.MaterialHandles;

            out << YAML::EndMap; // MeshComponent
        }

        if (e.HasComponent<CameraComponent>()) {
            auto& camera = e.GetComponent<CameraComponent>();

            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // CameraComponent

            out << YAML::Key << "FOV" << YAML::Value << camera.FOV;
            out << YAML::Key << "Near" << YAML::Value << camera.Near;
            out << YAML::Key << "Far" << YAML::Value << camera.Far;

            out << YAML::EndMap; // CameraComponent
        }

        if (e.HasComponent<DirectionalLightComponent>()) {
            auto& light = e.GetComponent<DirectionalLightComponent>();

            out << YAML::Key << "DirectionalLightComponent";
            out << YAML::BeginMap; // DirectionalLightComponent

            out << YAML::Key << "Color" << YAML::Value << light.Color;
            out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;

            out << YAML::EndMap; // DirectionalLightComponent
        }

        if (e.HasComponent<PointLightComponent>()) {
            auto& light = e.GetComponent<PointLightComponent>();

            out << YAML::Key << "PointLightComponent";
            out << YAML::BeginMap; // PointLightComponent

            out << YAML::Key << "Color" << YAML::Value << light.Color;
            out << YAML::Key << "Radius" << YAML::Value << light.Radius;
            out << YAML::Key << "Intensity" << YAML::Value << light.Intensity;

            out << YAML::EndMap; // PointLightComponent
        }

        if (e.HasComponent<EnviromentComponent>()) {
            auto& env = e.GetComponent<EnviromentComponent>();

            out << YAML::Key << "EnviromentComponent";
            out << YAML::BeginMap; // EnviromentComponent

            out << YAML::Key << "EnviromentMap" << YAML::Value << env.EnviromentMap;
            out << YAML::Key << "LevelOfDetail" << YAML::Value << env.LevelOfDetail;

            out << YAML::EndMap; // EnviromentComponent
        }

        out << YAML::EndMap; // Entity
    }

    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene) {}

    void SceneSerializer::Serialize(const FS::Path& path) {
        if (m_Scene->GetEntities().size() == 0) return;
        YAML::Emitter out;

        out << YAML::BeginMap;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        // entities
        for (auto& id : m_Scene->GetEntities()) {
            Entity entity(id, m_Scene);
            SerializeEntity(out, entity);
        }

        out << YAML::EndSeq << YAML::EndMap;

        std::ofstream stream(path);
        stream << out.c_str();
    }

    void SceneSerializer::Deserialize(const FS::Path& path) {
        std::string contents = Util::ReadEntireFile(path);

        YAML::Node node = YAML::Load(contents.c_str());

        if (!node["Entities"]) return;

        YAML::Node entities = node["Entities"];

        for (auto entity : entities) {
            Entity e;

            // Still hate how this works
            if (entity["TagComponent"]) {
                auto yamlTag = entity["TagComponent"];

                e = Entity(m_Scene->CreateEntityWithUUID(yamlTag["UUID"].as<u64>()), m_Scene);
                TagComponent& tag = e.GetComponent<TagComponent>();
                tag.Name = yamlTag["Name"].as<std::string>();
            }

            if (entity["TransformComponent"]) {
                auto yamlTransform = entity["TransformComponent"];

                TransformComponent transform;
                transform.Position = yamlTransform["Position"].as<BlVec3<f32>>();
                transform.Rotation = yamlTransform["Rotation"].as<BlVec3<f32>>();
                transform.Scale = yamlTransform["Scale"].as<BlVec3<f32>>();

                e.AddComponent<TransformComponent>(transform);
            }

            if (entity["MeshComponent"]) {
                auto yamlMesh = entity["MeshComponent"];

                MeshComponent mesh;
                mesh.MeshHandle = yamlMesh["MeshHandle"].as<u64>();
                mesh.MaterialHandles = yamlMesh["MaterialHandles"].as<std::map<u32, u64>>();

                e.AddComponent<MeshComponent>(mesh);
            }

            if (entity["CameraComponent"]) {
                auto yamlCamera = entity["CameraComponent"];

                CameraComponent camera;
                camera.FOV = yamlCamera["FOV"].as<f32>();
                camera.Near = yamlCamera["Near"].as<f32>();
                camera.Far = yamlCamera["Far"].as<f32>();

                e.AddComponent<CameraComponent>(camera);
            }

            if (entity["DirectionalLightComponent"]) {
                auto yamlLight = entity["DirectionalLightComponent"];

                DirectionalLightComponent light;
                light.Color = yamlLight["Color"].as<BlVec3<f32>>();
                light.Intensity = yamlLight["Intensity"].as<f32>();

                e.AddComponent<DirectionalLightComponent>(light);
            }

            if (entity["PointLightComponent"]) {
                auto yamlLight = entity["PointLightComponent"];

                PointLightComponent light;
                light.Color = yamlLight["Color"].as<BlVec3<f32>>();
                light.Radius = yamlLight["Radius"].as<f32>();
                light.Intensity = yamlLight["Intensity"].as<f32>();

                e.AddComponent<PointLightComponent>(light);
            }

            if (entity["EnviromentComponent"]) {
                auto yamlEnv = entity["EnviromentComponent"];

                EnviromentComponent env;
                env.EnviromentMap = yamlEnv["EnviromentMap"].as<u64>();
                env.LevelOfDetail = yamlEnv["LevelOfDetail"].as<f32>();

                e.AddComponent<EnviromentComponent>(env);
            }
        }
    }

} // namespace Blackberry