#pragma once

#include "blackberry/ecs/ecs.hpp"
#include "blackberry/scene/scene.hpp"

namespace Blackberry {

    class Entity {
    public:
        Entity() = default;
        explicit Entity(EntityID id, Scene* scene)
            : ID(id), EntityScene(scene) {}

        template <typename T>
        void AddComponent(const T& component = T{}) {
            EntityScene->m_ECS->AddComponent<T>(ID, component);
        }

        template <typename T>
        bool HasComponent() {
            return EntityScene->m_ECS->HasComponent<T>(ID);
        }

        template <typename T>
        T& GetComponent() {
            return EntityScene->m_ECS->GetComponent<T>(ID);
        }

        template <typename T>
        void RemoveComponent() {
            EntityScene->m_ECS->RemoveComponent<T>(ID);
        }

    public:
        EntityID ID = entt::null;
        Scene* EntityScene = nullptr;
    };

} // namespace Blackberry