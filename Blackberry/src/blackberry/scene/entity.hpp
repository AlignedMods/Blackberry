#pragma once

#include "blackberry/ecs/ecs.hpp"
#include "blackberry/scene/scene.hpp"

namespace Blackberry {

    class Entity {
    public:
        Entity() = default;
        explicit Entity(EntityID id, Scene* scene)
            : m_ID(id), m_Scene(scene) {}

        template <typename T>
        void AddComponent(const T& component) {
            m_Scene->m_Coordinator->AddComponent<T>(m_ID, component);
        }

        template <typename T>
        bool HasComponent() {
            return m_Scene->m_Coordinator->HasComponent<T>(m_ID);
        }

    private:
        EntityID m_ID = 0;
        Scene* m_Scene = nullptr;
    };

} // namespace Blackberry