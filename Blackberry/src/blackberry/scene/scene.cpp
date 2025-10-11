#include "blackberry/scene/scene.hpp"
#include "blackberry/ecs/ecs.hpp"

namespace Blackberry {

    Scene::Scene()
        : m_Coordinator(new Coordinator) {}

    void Scene::OnUpdate() {
        m_Coordinator->Update();
    }

    void Scene::OnRender() {
        m_Coordinator->Render();
    }

    EntityID Scene::CreateEntity(const std::string& name) {
        u64 id = UUID();
        CreateEntityWithUUID(id);
        m_NamedEntityMap[name] = id;

        Components::Tag& tag = m_Coordinator->GetComponent<Components::Tag>(m_EntityMap.at(id));
        tag.Name = name;
        
        return m_EntityMap.at(id);
    }

    EntityID Scene::CreateEntityWithUUID(u64 uuid) {
        m_EntityMap[uuid] = m_Coordinator->CreateEntity();

        m_Coordinator->AddComponent<Components::Tag>(m_EntityMap.at(uuid), { "", uuid });

        return m_EntityMap.at(uuid);
    }

    EntityID Scene::GetEntity(const std::string& name) {
        if (!m_NamedEntityMap.contains(name)) {
            CreateEntity(name);
        }

        return m_EntityMap.at(m_NamedEntityMap.at(name));
    }

    std::vector<EntityID>& Scene::GetEntities() {
        return m_Coordinator->GetEntities();
    }

} // namespace Blackberry
