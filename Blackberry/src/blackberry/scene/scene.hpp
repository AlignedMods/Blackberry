#pragma once

#include "blackberry/ecs/ecs.hpp"

#include <unordered_map>

namespace Blackberry {

    class Scene {
    public:
        Scene();

        void OnUpdate();
        void OnRender();

        EntityID CreateEntity(const std::string& name);
        EntityID GetEntity(const std::string& name);

    private:
        Coordinator* m_Coordinator = nullptr; // ECS coordinator
        std::unordered_map<std::string, EntityID> m_EntityMap;

        friend class Entity;
    };

} // namespace Blackberry
