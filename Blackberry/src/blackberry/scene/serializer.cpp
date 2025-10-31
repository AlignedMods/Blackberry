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
            using namespace Components;

            Entity entity(id, m_Scene);
            std::string name = "ID - " + std::to_string(static_cast<u32>(id));
            
            if (entity.HasComponent<Tag>()) {
                Tag& tag = entity.GetComponent<Tag>();

                j["Entities"][name]["TagComponent"] = { 
                    {"Name", tag.Name},
                    {"UUID", tag.UUID}
                };
            }

            if (entity.HasComponent<Transform>()) {
                Transform& transform = entity.GetComponent<Transform>();

                j["Entities"][name]["TransformComponent"] = { 
                    {"Position", {transform.Position.x, transform.Position.y, transform.Position.z} },
                    {"Dimensions", {transform.Dimensions.x, transform.Dimensions.y} }
                };
            }

            if (entity.HasComponent<Drawable>()) {
                Drawable& drawable = entity.GetComponent<Drawable>();

                j["Entities"][name]["DrawableComponent"] = { 
                    {"Color", {drawable.Color.r, drawable.Color.g, drawable.Color.b, drawable.Color.a}},
                    {"ShapeType", static_cast<u16>(drawable.ShapeType) }
                };
            }

            if (entity.HasComponent<Material>()) {
                Material& material = entity.GetComponent<Material>();

                j["Entities"][name]["MaterialComponent"] = { 
                    {"TextureHandle", material.TextureHandle},
                    {"Area", {material.Area.x, material.Area.y, material.Area.w, material.Area.h}}
                };
            }

            if (entity.HasComponent<Script>()) {
                Script& script = entity.GetComponent<Script>();

                j["Entities"][name]["ScriptComponent"] = { 
                    {"ModulePath", script.ModulePath.string()}
                };
            }

            if (entity.HasComponent<Velocity>()) {
                Velocity& velocity = entity.GetComponent<Velocity>();

                j["Entities"][name]["VelocityComponent"] = { 
                    {"Acceleration", {velocity.Acceleration.x, velocity.Acceleration.y}}
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
        using namespace Components;

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
                Tag& entityTag = entity.GetComponent<Tag>();
                entityTag.Name = jsonTag.at("Name");
            }

            // TransformComponent
            if (jsonEntity.contains("TransformComponent")) {
                auto& jsonTransform = jsonEntity.at("TransformComponent");
                std::array<f32, 3> position = jsonTransform.at("Position");
                std::array<f32, 2> dimensions = jsonTransform.at("Dimensions");
                
                entity.AddComponent<Transform>({ BlVec3(position[0], position[1], position[2]), 0.0f, BlVec2(dimensions[0], dimensions[1]) });
            }

            // DrawableComponent
            if (jsonEntity.contains("DrawableComponent")) {
                auto& jsonDrawable = jsonEntity.at("DrawableComponent");
                std::array<u8, 4> color = jsonDrawable.at("Color");
                u16 shapeType = jsonDrawable.at("ShapeType");

                entity.AddComponent<Drawable>({ BlColor(color[0], color[1], color[2], color[3]), static_cast<Shape>(shapeType) });
            }

            // MaterialComponent
            if (jsonEntity.contains("MaterialComponent")) {
                auto& jsonMaterial = jsonEntity.at("MaterialComponent");
                u64 textureHandle = jsonMaterial.at("TextureHandle");
                std::array<f32, 4> area = jsonMaterial.at("Area");

                BlTexture tex = std::get<BlTexture>(m_Scene->GetAssetManager().GetAsset(textureHandle).Data);

                entity.AddComponent<Material>({ textureHandle, BlRec(area[0], area[1], area[2], area[3]) });
            }

            if (jsonEntity.contains("ScriptComponent")) {
                auto& jsonScript = jsonEntity.at("ScriptComponent");
                std::filesystem::path modulePath = jsonScript.at("ModulePath");
                std::filesystem::path filePath = m_AssetDirectory / modulePath;
                entity.AddComponent<Script>({ modulePath, filePath });
            }

            if (jsonEntity.contains("VelocityComponent")) {
                auto& jsonVelocity = jsonEntity.at("VelocityComponent");
                std::array<f32, 2> acceleration = jsonVelocity.at("Acceleration");
                entity.AddComponent<Velocity>({ BlVec2(acceleration[0], acceleration[1]) });
            }
        }
    }

} // namespace Blackberry