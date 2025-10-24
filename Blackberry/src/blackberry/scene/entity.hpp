#pragma once

#include "blackberry/ecs/ecs.hpp"
#include "blackberry/scene/scene.hpp"

namespace Blackberry {

    class Entity {
    public:
        Entity() = default;
        explicit Entity(EntityID id, Scene* scene)
            : ID(id), m_Scene(scene) {}

        template <typename T>
        void AddComponent(const T& component = T{}) {
            m_Scene->m_ECS->AddComponent<T>(ID, component);
        }

        template <typename T>
        bool HasComponent() {
            return m_Scene->m_ECS->HasComponent<T>(ID);
        }

        template <typename T>
        T& GetComponent() {
            return m_Scene->m_ECS->GetComponent<T>(ID);
        }

        template <typename T>
        void RemoveComponent() {
            m_Scene->m_ECS->RemoveComponent<T>(ID);
        }

    public:
        EntityID ID = entt::null;

    private:
        Scene* m_Scene = nullptr;
    };

} // namespace Blackberry