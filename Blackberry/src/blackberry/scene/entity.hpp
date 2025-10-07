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
            m_Scene->m_Coordinator->AddComponent<T>(ID, component);
        }

        template <typename T>
        bool HasComponent() {
            return m_Scene->m_Coordinator->HasComponent<T>(ID);
        }

        template <typename T>
        T& GetComponent() {
            return m_Scene->m_Coordinator->GetComponent<T>(ID);
        }

    public:
        EntityID ID = 0;

    private:
        Scene* m_Scene = nullptr;
    };

} // namespace Blackberry