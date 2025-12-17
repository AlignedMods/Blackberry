#include "blackberry/scene/scene_serializer.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "json.hpp"
using json = nlohmann::json;

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

    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene) {}

    void SceneSerializer::Serialize(const FS::Path& path) {
        if (m_Scene->GetEntities().size() == 0) return;
        json j;

        // entities
        for (auto& id : m_Scene->GetEntities()) {
            Entity entity(id, m_Scene);
            std::string name = "ID - " + std::to_string(static_cast<u32>(id));
            
            if (entity.HasComponent<TagComponent>()) {
                TagComponent& tag = entity.GetComponent<TagComponent>();

                j["Entities"][name]["TagComponent"] = { 
                    {"Name", tag.Name},
                    {"UUID", tag.UUID}
                };
            }

            if (entity.HasComponent<TransformComponent>()) {
                TransformComponent& transform = entity.GetComponent<TransformComponent>();

                j["Entities"][name]["TransformComponent"] = { 
                    {"Position", {transform.Position.x, transform.Position.y, transform.Position.z} },
                    {"Rotation", {transform.Rotation.x, transform.Rotation.y, transform.Rotation.z} },
                    {"Scale",    {transform.Scale.x,    transform.Scale.y,    transform.Scale.z   } }
                };
            }

            if (entity.HasComponent<MeshComponent>()) {
                MeshComponent& mesh = entity.GetComponent<MeshComponent>();

                j["Entities"][name]["MeshComponent"] = { 
                    {"MeshHandle", mesh.MeshHandle}
                };

                if (Project::GetAssetManager().ContainsAsset(mesh.MeshHandle)) {
                    Model& model = std::get<Model>(Project::GetAssetManager().GetAsset(mesh.MeshHandle).Data);

                    for (u32 i = 0; i < model.MeshCount; i++) {
                        j["Entities"][name]["MeshComponent"]["Materials"][i] = model.Meshes[i].MaterialHandle;
                    }
                } else {
                    j["Entities"][name]["MeshComponent"]["Materials"] = json::array();
                }
            }

            if (entity.HasComponent<CameraComponent>()) {
                CameraComponent& camera = entity.GetComponent<CameraComponent>();

                j["Entities"][name]["CameraComponent"] = {
                    {"Near", camera.Near},
                    {"Far", camera.Far}
                };
            }

            if (entity.HasComponent<ScriptComponent>()) {
                ScriptComponent& script = entity.GetComponent<ScriptComponent>();

                j["Entities"][name]["ScriptComponent"] = { 
                    {"ModulePath", script.ModulePath.String()}
                };
            }

            if (entity.HasComponent<RigidBodyComponent>()) {
                RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();

                j["Entities"][name]["RigidBodyComponent"] = {
                    {"Type", RigidBodyTypeToString(rigidBody.Type)},
                    {"Mass", rigidBody.Mass},
                    {"EnableGravity", rigidBody.EnableGravity}
                };
            }

            if (entity.HasComponent<ColliderComponent>()) {
                ColliderComponent& collider = entity.GetComponent<ColliderComponent>();

                j["Entities"][name]["ColliderComponent"] = {
                    {"Type", ColliderTypeToString(collider.Type)},
                    {"Scale", {collider.Scale.x, collider.Scale.y, collider.Scale.z}}
                };
            }

            if (entity.HasComponent<TextComponent>()) {
                TextComponent& text = entity.GetComponent<TextComponent>();

                j["Entities"][name]["TextComponent"] = {
                    {"Contents", text.Contents},
                    {"FontHandle", text.FontHandle},
                    {"Kerning", text.Kerning},
                    {"LineSpacing", text.LineSpacing}
                };
            }

            if (entity.HasComponent<DirectionalLightComponent>()) {
                DirectionalLightComponent& light = entity.GetComponent<DirectionalLightComponent>();

                j["Entities"][name]["DirectionalLightComponent"] = {
                    {"Color", {light.Color.x, light.Color.y, light.Color.z}},
                    {"Intensity", light.Intensity}
                };
            }

            if (entity.HasComponent<PointLightComponent>()) {
                PointLightComponent& light = entity.GetComponent<PointLightComponent>();

                j["Entities"][name]["PointLightComponent"] = {
                    {"Color", {light.Color.x, light.Color.y, light.Color.z}},
                    {"Radius", light.Radius},
                    {"Intensity", light.Intensity}
                };
            }

            if (entity.HasComponent<EnviromentComponent>()) {
                EnviromentComponent& env = entity.GetComponent<EnviromentComponent>();

                j["Entities"][name]["EnviromentComponent"] = {
                    {"EnviromentMap", env.EnviromentMap}
                };
            }
        }

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void SceneSerializer::Deserialize(const FS::Path& path) {
        std::string contents = Util::ReadEntireFile(path);

        json j = json::parse(contents);
        auto& entities = j.at("Entities");

        // entities
        for (auto& jsonEntity : entities) {
            Entity entity;

            // TagComponent
            // (aligned) TODO: potentially redo the way tags get handled?
            if (jsonEntity.contains("TagComponent")) {
                auto& jsonTag = jsonEntity.at("TagComponent");

                entity = Entity(m_Scene->CreateEntityWithUUID(jsonTag.at("UUID")), m_Scene);
                TagComponent& entityTag = entity.GetComponent<TagComponent>();
                entityTag.Name = jsonTag.at("Name");
            }

            // Transform2DComponent
            if (jsonEntity.contains("TransformComponent")) {
                auto& jsonTransform = jsonEntity.at("TransformComponent");
                std::array<f32, 3> position = jsonTransform.at("Position");
                std::array<f32, 3> rotation = jsonTransform.at("Rotation");
                std::array<f32, 3> scale = jsonTransform.at("Scale");
                
                entity.AddComponent<TransformComponent>({ BlVec3(position[0], position[1], position[2]), BlVec3(rotation[0], rotation[1], rotation[2]), BlVec3(scale[0], scale[1], scale[2]) });
            }

            // MeshComponent
            if (jsonEntity.contains("MeshComponent")) {
                auto& jsonMesh = jsonEntity.at("MeshComponent");
                u64 meshHandle = jsonMesh.at("MeshHandle");
                std::vector<u64> materials = jsonMesh.at("Materials");

                // set materials
                if (Project::GetAssetManager().ContainsAsset(meshHandle)) {
                    Model& model = std::get<Model>(Project::GetAssetManager().GetAsset(meshHandle).Data);

                    for (u32 i = 0; i < model.MeshCount; i++) {
                        model.Meshes[i].MaterialHandle = materials[i];
                    }
                }

                entity.AddComponent<MeshComponent>({ meshHandle });
            }

            if (jsonEntity.contains("CameraComponent")) {
                auto& jsonCamera = jsonEntity.at("CameraComponent");
                CameraComponent cam;

                f32 nearZ = jsonCamera.at("Near");
                f32 farZ = jsonCamera.at("Far");

                cam.Near = nearZ;
                cam.Far = farZ;
                cam.Active = true;

                entity.AddComponent<CameraComponent>(cam);
            }

            if (jsonEntity.contains("ScriptComponent")) {
                auto& jsonScript = jsonEntity.at("ScriptComponent");
                std::string modulePath = jsonScript.at("ModulePath");
                FS::Path filePath = Project::GetAssetPath(modulePath);
                entity.AddComponent<ScriptComponent>({ modulePath, filePath });
            }

            if (jsonEntity.contains("RigidBodyComponent")) {
                auto& jsonRigidBody = jsonEntity.at("RigidBodyComponent");
                std::string type = jsonRigidBody.at("Type");
                f32 mass = jsonRigidBody.at("Mass");

                entity.AddComponent<RigidBodyComponent>({ StringToRigidBodyType(type), BlVec3(), BlVec3(), BlVec3(), BlVec3(), mass });
            }

            if (jsonEntity.contains("ColliderComponent")) {
                auto& jsonCollider = jsonEntity.at("ColliderComponent");
                std::string type = jsonCollider.at("Type");
                std::array<f32, 3> scale = jsonCollider.at("Scale");

                entity.AddComponent<ColliderComponent>({ StringToColliderType(type), BlVec3(scale[0], scale[1], scale[2]) });
            }

            if (jsonEntity.contains("TextComponent")) {
                auto& jsonText = jsonEntity.at("TextComponent");
                std::string contents = jsonText.at("Contents");
                u64 fontHandle = jsonText.at("FontHandle");
                f32 kerning = jsonText.at("Kerning");
                f32 lineSpacing = jsonText.at("LineSpacing");

                entity.AddComponent<TextComponent>({contents, fontHandle, kerning, lineSpacing});
            }

            if (jsonEntity.contains("DirectionalLightComponent")) {
                auto& jsonLight = jsonEntity.at("DirectionalLightComponent");

                std::array<f32, 3> color = jsonLight.at("Color");
                f32 intensity = jsonLight.at("Intensity");

                DirectionalLightComponent light;
                light.Color = BlVec3<f32>(color[0], color[1], color[2]);
                light.Intensity = intensity;

                entity.AddComponent<DirectionalLightComponent>(light);
            }

            if (jsonEntity.contains("PointLightComponent")) {
                auto& jsonLight = jsonEntity.at("PointLightComponent");

                std::array<f32, 3> color = jsonLight.at("Color");
                f32 radius = jsonLight.at("Radius");
                f32 intensity = jsonLight.at("Intensity");

                entity.AddComponent<PointLightComponent>({BlVec3(color[0], color[1], color[2]), radius, intensity});
            }

            if (jsonEntity.contains("EnviromentComponent")) {
                auto& jsonEnv = jsonEntity.at("EnviromentComponent");

                u64 envMapHandle = jsonEnv.at("EnviromentMap");

                entity.AddComponent<EnviromentComponent>({envMapHandle});
            }
        }
    }

} // namespace Blackberry