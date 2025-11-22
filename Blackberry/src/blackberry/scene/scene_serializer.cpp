#include "blackberry/scene/scene_serializer.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/project/project.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    SceneSerializer::SceneSerializer(Scene* scene)
        : m_Scene(scene) {}

    void SceneSerializer::Serialize(const std::filesystem::path& path) {
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

            if (entity.HasComponent<Transform2DComponent>()) {
                Transform2DComponent& transform = entity.GetComponent<Transform2DComponent>();

                j["Entities"][name]["Transform2DComponent"] = { 
                    {"Position", {transform.Position.x, transform.Position.y, transform.Position.z} },
                    {"Rotation", transform.Rotation },
                    {"Dimensions", {transform.Dimensions.x, transform.Dimensions.y} }
                };
            }

            if (entity.HasComponent<ShapeRendererComponent>()) {
                ShapeRendererComponent& shapeRenderer = entity.GetComponent<ShapeRendererComponent>();

                j["Entities"][name]["ShapeRendererComponent"] = { 
                    {"Color", {shapeRenderer.Color.r, shapeRenderer.Color.g, shapeRenderer.Color.b, shapeRenderer.Color.a}},
                    {"ShapeType", static_cast<u16>(shapeRenderer.Shape) }
                };
            }

            if (entity.HasComponent<SpriteRendererComponent>()) {
                SpriteRendererComponent& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                j["Entities"][name]["SpriteRendererComponent"] = { 
                    {"Color", {spriteRenderer.Color.r, spriteRenderer.Color.g, spriteRenderer.Color.b, spriteRenderer.Color.a}},
                    {"ShapeType", static_cast<u16>(spriteRenderer.Shape) },
                    {"TextureHandle", spriteRenderer.TextureHandle},
                    {"TextureArea", {spriteRenderer.Area.x, spriteRenderer.Area.y, spriteRenderer.Area.w, spriteRenderer.Area.h}}
                };
            }

            if (entity.HasComponent<CameraComponent>()) {
                CameraComponent& camera = entity.GetComponent<CameraComponent>();
                SceneCamera& cam = camera.Camera;

                j["Entities"][name]["CameraComponent"] = {
                    {"Position", {cam.Position.x, cam.Position.y}},
                    {"Offset", {cam.Offset.x, cam.Offset.y}},
                    {"Rotation", cam.Rotation},
                    {"Near", cam.Near},
                    {"Far", cam.Far},
                    {"Size", {cam.Size.x, cam.Size.y}}
                };
            }

            if (entity.HasComponent<ScriptComponent>()) {
                ScriptComponent& script = entity.GetComponent<ScriptComponent>();

                j["Entities"][name]["ScriptComponent"] = { 
                    {"ModulePath", script.ModulePath.string()}
                };
            }

            if (entity.HasComponent<RigidBodyComponent>()) {
                RigidBodyComponent& rigidBody = entity.GetComponent<RigidBodyComponent>();

                j["Entities"][name]["RigidBodyComponent"] = { 
                    {"Velocity", {rigidBody.Velocity.x, rigidBody.Velocity.y}},
                    {"Acceleration", {rigidBody.Acceleration.x, rigidBody.Acceleration.y}},
                    {"Force", {rigidBody.Force.x, rigidBody.Force.y}},
                    {"Mass", rigidBody.Mass}
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
        }

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void SceneSerializer::Deserialize(const std::filesystem::path& path) {
        std::string contents = ReadEntireFile(path);

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
            if (jsonEntity.contains("Transform2DComponent")) {
                auto& jsonTransform = jsonEntity.at("Transform2DComponent");
                std::array<f32, 3> position = jsonTransform.at("Position");
                std::array<f32, 2> dimensions = jsonTransform.at("Dimensions");
                f32 rotation = jsonTransform.at("Rotation");
                
                entity.AddComponent<Transform2DComponent>({ BlVec3(position[0], position[1], position[2]), rotation, BlVec2(dimensions[0], dimensions[1]) });
            }

            // ShapeRendererComponent
            if (jsonEntity.contains("ShapeRendererComponent")) {
                auto& jsonShapeRenderer = jsonEntity.at("ShapeRendererComponent");
                std::array<u8, 4> color = jsonShapeRenderer.at("Color");
                u16 shapeType = jsonShapeRenderer.at("ShapeType");

                entity.AddComponent<ShapeRendererComponent>({ BlColor(color[0], color[1], color[2], color[3]), static_cast<ShapeType>(shapeType) });
            }

            // SpriteRendererComponent
            if (jsonEntity.contains("SpriteRendererComponent")) {
                auto& jsonSpriteRenderer = jsonEntity.at("SpriteRendererComponent");
                std::array<u8, 4> color = jsonSpriteRenderer.at("Color");
                u16 shapeType = jsonSpriteRenderer.at("ShapeType");
                u64 textureHandle = jsonSpriteRenderer.at("TextureHandle");
                std::array<f32, 4> textureArea = jsonSpriteRenderer.at("TextureArea");

                entity.AddComponent<SpriteRendererComponent>({ BlColor(color[0], color[1], color[2], color[3]), static_cast<ShapeType>(shapeType), 
                                                      textureHandle, BlRec(textureArea[0], textureArea[1], textureArea[2], textureArea[3]) });
            }

            if (jsonEntity.contains("CameraComponent")) {
                auto& jsonCamera = jsonEntity.at("CameraComponent");
                SceneCamera cam;

                std::array<f32, 2> position = jsonCamera.at("Position");
                std::array<f32, 2> offset = jsonCamera.at("Offset");
                std::array<f32, 2> size = jsonCamera.at("Size");
                f32 rotation = jsonCamera.at("Rotation");
                f32 nearZ = jsonCamera.at("Near");
                f32 farZ = jsonCamera.at("Far");

                cam.Position = BlVec2(position[0], position[1]);
                cam.Offset = BlVec2(offset[0], offset[1]);
                cam.Size = BlVec2(size[0], size[1]);
                cam.Rotation = rotation;
                cam.Near = nearZ;
                cam.Far = farZ;

                entity.AddComponent<CameraComponent>({ cam, true });
            }

            if (jsonEntity.contains("ScriptComponent")) {
                auto& jsonScript = jsonEntity.at("ScriptComponent");
                std::filesystem::path modulePath = jsonScript.at("ModulePath");
                std::filesystem::path filePath = Project::GetAssetPath(modulePath);
                entity.AddComponent<ScriptComponent>({ modulePath, filePath });
            }

            if (jsonEntity.contains("RigidBodyComponent")) {
                auto& jsonRigidBody = jsonEntity.at("RigidBodyComponent");
                std::array<f32, 2> velocity = jsonRigidBody.at("Velocity");
                std::array<f32, 2> acceleration = jsonRigidBody.at("Acceleration");
                std::array<f32, 2> force = jsonRigidBody.at("Force");
                f32 mass = jsonRigidBody.at("Mass");

                entity.AddComponent<RigidBodyComponent>({ BlVec2(velocity[0], velocity[1]), BlVec2(acceleration[0], acceleration[1]), BlVec2(force[0], force[1]), mass });
            }

            if (jsonEntity.contains("TextComponent")) {
                auto& jsonText = jsonEntity.at("TextComponent");
                std::string contents = jsonText.at("Contents");
                u64 fontHandle = jsonText.at("FontHandle");
                f32 kerning = jsonText.at("Kerning");
                f32 lineSpacing = jsonText.at("LineSpacing");

                entity.AddComponent<TextComponent>({contents, fontHandle, kerning, lineSpacing});
            }
        }
    }

} // namespace Blackberry