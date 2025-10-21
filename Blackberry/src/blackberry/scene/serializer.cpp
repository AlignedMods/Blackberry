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
                    {"Position", {transform.Position.x, transform.Position.y} },
                    {"Dimensions", {transform.Dimensions.x, transform.Dimensions.y} }
                };
            }

            if (entity.HasComponent<Drawable>()) {
                Drawable& drawable = entity.GetComponent<Drawable>();

                j["Entities"][name]["DrawableComponent"] = { {"Color", {drawable.Color.r, drawable.Color.g, drawable.Color.b, drawable.Color.a}} };
            }

            if (entity.HasComponent<Material>()) {
                Material& material = entity.GetComponent<Material>();

                j["Entities"][name]["MaterialComponent"] = { 
                    {"TexturePath", material.TexturePath.string()},
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

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void SceneSerializer::Deserialize(const std::filesystem::path& path) {
        using namespace Components;

        std::string contents = ReadEntireFile(path);

        json j = json::parse(contents);
        auto& entities = j.at("Entities");

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
                std::array<f32, 2> position = jsonTransform.at("Position");
                std::array<f32, 2> dimensions = jsonTransform.at("Dimensions");
                
                entity.AddComponent<Transform>({ BlVec2(position[0], position[1]), BlVec2(dimensions[0], dimensions[1]) });
            }

            // DrawableComponent
            if (jsonEntity.contains("DrawableComponent")) {
                auto& jsonDrawable = jsonEntity.at("DrawableComponent");
                std::array<u8, 4> color = jsonDrawable.at("Color");

                entity.AddComponent<Drawable>({ BlColor(color[0], color[1], color[2], color[3]) });
            }

            // MaterialComponent
            if (jsonEntity.contains("MaterialComponent")) {
                auto& jsonMaterial = jsonEntity.at("MaterialComponent");
                std::filesystem::path filePath = jsonMaterial.at("TexturePath");
                std::array<f32, 4> area = jsonMaterial.at("Area");

                BlTexture tex;
                tex.Create(m_AssetDirectory / filePath);

                entity.AddComponent<Material>({ filePath, tex, BlRec(area[0], area[1], area[2], area[3]) });
            }

            if (jsonEntity.contains("ScriptComponent")) {
                auto& jsonScript = jsonEntity.at("ScriptComponent");
                std::filesystem::path modulePath = jsonScript.at("ModulePath");
                entity.AddComponent<Script>({ modulePath });
            }

            if (jsonEntity.contains("VelocityComponent")) {
                auto& jsonVelocity = jsonEntity.at("VelocityComponent");
                std::array<f32, 2> acceleration = jsonVelocity.at("Acceleration");
                entity.AddComponent<Velocity>({ BlVec2(acceleration[0], acceleration[1]) });
            }
        }
    }

} // namespace Blackberry