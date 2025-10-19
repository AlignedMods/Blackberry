#include "blackberry/scene/scene.hpp"
#include "blackberry/ecs/ecs.hpp"

namespace Blackberry {

    Scene::Scene()
        : m_Coordinator(new Coordinator) {
        BL_INFO("Scene created.");
    }

    Scene::~Scene() {
        // Delete();
        BL_INFO("Scene destroyed.");
    }

    template <typename T>
    static void CopyComponent(Coordinator* dest, Coordinator* src, EntityID destEntity, EntityID srcEntity) {
        if (src->HasComponent<T>(srcEntity)) {
            T& component = src->GetComponent<T>(srcEntity);
            dest->AddComponent<T>(destEntity, component);
        }
    }

    Scene* Scene::Copy(Scene* current) {
        Scene* scene = new Scene();

        for (auto&[uuid, entity] : current->m_EntityMap) {
            scene->CreateEntityWithUUID(uuid);

            EntityID otherEntity = scene->m_EntityMap.at(uuid);

            // copy components
            using namespace Components;
            CopyComponent<Tag>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
            CopyComponent<Transform>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
            CopyComponent<Material>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
            CopyComponent<Drawable>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
            CopyComponent<Text>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
            CopyComponent<Script>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
            CopyComponent<Velocity>(scene->m_Coordinator, current->m_Coordinator, otherEntity, entity);
        }

        return scene;
    }

    void Scene::Delete() {}

    void Scene::OnUpdate() {
        m_Coordinator->Update();
    }

    void Scene::OnRuntimeUpdate() {
        m_Coordinator->RuntimeUpdate();
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
