#include "blackberry/scene/scene_serializer.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"
#include "blackberry/core/yaml_utils.hpp"

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

        if (e.HasComponent<RelationshipComponent>()) {
            auto& rel = e.GetComponent<RelationshipComponent>();

            out << YAML::Key << "RelationshipComponent";
            out << YAML::BeginMap; // RelationshipComponent

            out << YAML::Key << "Parent" << YAML::Value << rel.Parent;
            out << YAML::Key << "FirstChild" << YAML::Value << rel.FirstChild;
            out << YAML::Key << "NextSibling" << YAML::Value << rel.NextSibling;
            out << YAML::Key << "PrevSibling" << YAML::Value << rel.PrevSibling;

            out << YAML::EndMap; // RelationshipComponent
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

        if (e.HasComponent<RigidBodyComponent>()) {
            auto& rigidBody = e.GetComponent<RigidBodyComponent>();

            out << YAML::Key << "RigidBodyComponent";
            out << YAML::BeginMap; // RigidBodyComponent

            out << YAML::Key << "Type" << YAML::Value << RigidBodyTypeToString(rigidBody.Type);
            out << YAML::Key << "Resitution" << YAML::Value << rigidBody.Resitution;
            out << YAML::Key << "Friction" << YAML::Value << rigidBody.Friction;

            out << YAML::EndMap; // RigidBodyComponent
        }

        if (e.HasComponent<BoxColliderComponent>()) {
            auto& collider = e.GetComponent<BoxColliderComponent>();

            out << YAML::Key << "BoxColliderComponent";
            out << YAML::BeginMap; // BoxColliderComponent

            out << YAML::Key << "Scale" << YAML::Value << collider.Scale;

            out << YAML::EndMap; // BoxColliderComponent
        }

        if (e.HasComponent<SphereColliderComponent>()) {
            auto& collider = e.GetComponent<SphereColliderComponent>();

            out << YAML::Key << "SphereColliderComponent";
            out << YAML::BeginMap; // SphereColliderComponent

            out << YAML::Key << "Radius" << YAML::Value << collider.Radius;

            out << YAML::EndMap; // SphereColliderComponent
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
            out << YAML::Key << "EnableBloom" << YAML::Value << env.EnableBloom;
            out << YAML::Key << "BloomThreshold" << YAML::Value << env.BloomThreshold;

            out << YAML::EndMap; // EnviromentComponent
        }

        out << YAML::EndMap; // Entity
    }

    SceneSerializer::SceneSerializer(Ref<Scene> scene)
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

            if (entity["TagComponent"]) {
                auto yamlTag = entity["TagComponent"];

                e = Entity(m_Scene->CreateEntityWithUUID(yamlTag["UUID"].as<u64>()), m_Scene);
                auto& tag = e.GetComponent<TagComponent>();
                tag.Name = yamlTag["Name"].as<std::string>();
            }

            if (entity["RelationshipComponent"]) {
                auto yamlRel = entity["RelationshipComponent"];

                auto& rel = e.GetComponent<RelationshipComponent>();
                rel.Parent = yamlRel["Parent"].as<u64>();
                rel.FirstChild = yamlRel["FirstChild"].as<u64>();
                rel.NextSibling = yamlRel["NextSibling"].as<u64>();
                rel.PrevSibling = yamlRel["PrevSibling"].as<u64>();
            }

            if (entity["TagComponent"]) {
                auto yamlTag = entity["TagComponent"];

                auto& tag = e.GetComponent<TagComponent>();
                tag.Name = yamlTag["Name"].as<std::string>();
            }

            if (entity["TransformComponent"]) {
                auto yamlTransform = entity["TransformComponent"];

                TransformComponent transform;
                transform.Position = yamlTransform["Position"].as<BlVec3>();
                transform.Rotation = yamlTransform["Rotation"].as<BlQuat>();
                transform.Scale = yamlTransform["Scale"].as<BlVec3>();

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

            if (entity["RigidBodyComponent"]) {
                auto yamlRigidBody = entity["RigidBodyComponent"];

                RigidBodyComponent rigidBody;
                rigidBody.Type = StringToRigidBodyType(yamlRigidBody["Type"].as<std::string>());
                rigidBody.Resitution = yamlRigidBody["Resitution"].as<f32>();
                rigidBody.Friction = yamlRigidBody["Friction"].as<f32>();

                e.AddComponent<RigidBodyComponent>(rigidBody);
            }

            if (entity["BoxColliderComponent"]) {
                auto yamlCollider = entity["BoxColliderComponent"];

                BoxColliderComponent collider;
                collider.Scale = yamlCollider["Scale"].as<BlVec3>();

                e.AddComponent<BoxColliderComponent>(collider);
            }

            if (entity["SphereColliderComponent"]) {
                auto yamlCollider = entity["SphereColliderComponent"];

                SphereColliderComponent collider;
                collider.Radius = yamlCollider["Radius"].as<f32>();

                e.AddComponent<SphereColliderComponent>(collider);
            }


            if (entity["DirectionalLightComponent"]) {
                auto yamlLight = entity["DirectionalLightComponent"];

                DirectionalLightComponent light;
                light.Color = yamlLight["Color"].as<BlVec3>();
                light.Intensity = yamlLight["Intensity"].as<f32>();

                e.AddComponent<DirectionalLightComponent>(light);
            }

            if (entity["PointLightComponent"]) {
                auto yamlLight = entity["PointLightComponent"];

                PointLightComponent light;
                light.Color = yamlLight["Color"].as<BlVec3>();
                light.Radius = yamlLight["Radius"].as<f32>();
                light.Intensity = yamlLight["Intensity"].as<f32>();

                e.AddComponent<PointLightComponent>(light);
            }

            if (entity["EnviromentComponent"]) {
                auto yamlEnv = entity["EnviromentComponent"];

                EnviromentComponent env;
                env.EnviromentMap = yamlEnv["EnviromentMap"].as<u64>();
                env.LevelOfDetail = yamlEnv["LevelOfDetail"].as<f32>();
                env.EnableBloom = yamlEnv["EnableBloom"].as<bool>();
                env.BloomThreshold = yamlEnv["BloomThreshold"].as<f32>();

                e.AddComponent<EnviromentComponent>(env);
            }

            m_Scene->FinishEntityEdit(e.GetComponent<TagComponent>().UUID);
        }
    }

} // namespace Blackberry