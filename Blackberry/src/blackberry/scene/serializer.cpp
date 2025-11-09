#include "blackberry/scene/serializer.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/core/util.hpp"

#include "json.hpp"
using json = nlohmann::json;

namespace Blackberry {

    SceneSerializer::SceneSerializer(Scene* scene, const std::filesystem::path& assetDirectory)
        : m_Scene(scene), m_AssetDirectory(assetDirectory) {}

    void SceneSerializer::Serialize(const std::filesystem::path& path) {
        if (m_Scene->GetEntities().size() == 0) { return; }
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
        }

        // assets
        for (const auto&[handle, asset] : m_Scene->GetAssetManager().GetAllAssets()) {
            std::string name = "Handle - " + std::to_string(handle);

            j["Assets"][name] = {
                { "Handle", handle },
                { "AssetPath", asset.FilePath }
            };
        }

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void SceneSerializer::Deserialize(const std::filesystem::path& path) {
        std::string contents = ReadEntireFile(path);

        json j = json::parse(contents);
        auto& entities = j.at("Entities");
        auto& assets = j.at("Assets");

        // assets (NOTE: must happen before entity loading, incase of entities depending on texture handles)
        for (auto& jsonAsset : assets) {
            Asset asset;
            asset.Type = AssetType::Texture;

            std::filesystem::path path = jsonAsset.at("AssetPath");
            asset.FilePath = path;
            
            BlTexture tex;
            tex.Create(m_AssetDirectory / jsonAsset.at("AssetPath"));
            asset.Data = tex;

            m_Scene->GetAssetManager().AddAssetWithHandle(jsonAsset.at("Handle"), asset);
        }

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

            // TransformComponent
            if (jsonEntity.contains("TransformComponent")) {
                auto& jsonTransform = jsonEntity.at("TransformComponent");
                std::array<f32, 3> position = jsonTransform.at("Position");
                std::array<f32, 2> dimensions = jsonTransform.at("Dimensions");
                
                entity.AddComponent<TransformComponent>({ BlVec3(position[0], position[1], position[2]), 0.0f, BlVec2(dimensions[0], dimensions[1]) });
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

            if (jsonEntity.contains("ScriptComponent")) {
                auto& jsonScript = jsonEntity.at("ScriptComponent");
                std::filesystem::path modulePath = jsonScript.at("ModulePath");
                std::filesystem::path filePath = m_AssetDirectory / modulePath;
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
        }
    }

} // namespace Blackberry