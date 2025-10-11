#pragma once

#include "blackberry/ecs/ecs.hpp"
#include "blackberry/scene/uuid.hpp"

#include <unordered_map>
#include <string>

namespace Blackberry {

    class Scene {
    public:
        Scene();

        void OnUpdate();
        void OnRender();

        EntityID CreateEntity(const std::string& name);
        EntityID CreateEntityWithUUID(u64 uuid);
        EntityID GetEntity(const std::string& name);
        std::vector<EntityID>& GetEntities();

    private:
        Coordinator* m_Coordinator = nullptr; // ECS coordinator
        std::unordered_map<u64, EntityID> m_EntityMap;
        std::unordered_map<std::string, u64> m_NamedEntityMap;

        friend class Entity;
    };

} // namespace Blackberry
