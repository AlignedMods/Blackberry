#pragma once

#include "blackberry/ecs/ecs.hpp"

#include <unordered_map>
#include <string>

namespace Blackberry {

    using UUID = u64;

    class Scene {
    public:
        Scene();

        void OnUpdate();
        void OnRender();

        EntityID CreateEntity(const std::string& name);
        EntityID GetEntity(const std::string& name);
        std::vector<EntityID>& GetEntities();

    private:
        Coordinator* m_Coordinator = nullptr; // ECS coordinator
        std::unordered_map<std::string, EntityID> m_EntityMap;

        friend class Entity;
    };

} // namespace Blackberry
