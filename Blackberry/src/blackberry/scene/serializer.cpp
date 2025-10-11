#include "blackberry/scene/serializer.hpp"
#include "blackberry/scene/entity.hpp"

#include "json.hpp"
using json = nlohmann::json;

#include <fstream>

namespace Blackberry {

    Serializer::Serializer(Scene* scene)
        : m_Scene(scene) {}

    void Serializer::Serialize(const std::filesystem::path& path) {
        if (m_Scene->GetEntities().size() == 0) { return; }
        json j;

        for (auto& id : m_Scene->GetEntities()) {
            using namespace Components;

            Entity entity(id, m_Scene);
            std::string name = "ID - " + std::to_string(id);

            j["Entities"][name] = { { "ID", id } };
            
            if (entity.HasComponent<Tag>()) {
                Tag& tag = entity.GetComponent<Tag>();

                j["Entities"][name]["TagComponent"] = { {"Name", tag.Name} };
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
        }

        std::ofstream stream(path);
        stream << j.dump(4);
    }

    void Serializer::Deserialize(const std::filesystem::path& path) {
        std::ifstream stream(path);
        std::stringstream ss;
        ss << stream.rdbuf();
        std::string contents = ss.str();
        ss.flush();

        json j = json::parse(contents);
        auto& entities = j.at("Entities");

        for (auto& entity : entities) {
            
        }
    }

} // namespace Blackberry