#pragma once

#include "blackberry/scene/uuid.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/scene/physics_world.hpp"

#include <unordered_map>
#include <string>

namespace Blackberry {

    class Scene {
    public:
        Scene();
        ~Scene();

        static Scene* Copy(Scene* current);
        void Delete();

        void OnPlay();
        void OnStop();

        void SetCamera(SceneCamera* camera);

        void OnUpdate();
        void OnRuntimeUpdate();
        void OnRender();

        void RenderEntity(EntityID entity);

        EntityID CreateEntity(const std::string& name);
        EntityID CreateEntityWithUUID(u64 uuid);

        void DuplicateEntity(EntityID entity);
        void CopyEntity(EntityID entity);

        void DestroyEntity(u64 uuid);

        EntityID GetEntity(const std::string& name);
        std::vector<EntityID> GetEntities();

    private:
        ECS* m_ECS = nullptr;
        PhysicsWorld* m_PhysicsWorld = nullptr;
        SceneCamera* m_Camera = nullptr;
        std::unordered_map<u64, EntityID> m_EntityMap;
        std::unordered_map<std::string, u64> m_NamedEntityMap;

        const f32 m_Gravity = 9.8f;

        friend class Entity;
    };

} // namespace Blackberry
