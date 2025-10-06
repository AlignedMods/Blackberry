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
        m_EntityMap[name] = m_Coordinator->CreateEntity();
        
        return m_EntityMap.at(name);
    }

    EntityID Scene::GetEntity(const std::string& name) {
        if (!m_EntityMap.contains(name)) {
            CreateEntity(name);
        }

        return m_EntityMap.at(name);
    }

} // namespace Blackberry
