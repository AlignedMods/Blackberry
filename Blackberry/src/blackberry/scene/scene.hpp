#pragma once

#include "blackberry/scene/uuid.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/assets/asset_manager.hpp"

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

        void OnUpdate();
        void OnRuntimeUpdate();
        void OnRender();

        void RenderEntity(EntityID entity, Blackberry::Components::Transform& transform, Blackberry::Components::Drawable& drawable);

        EntityID CreateEntity(const std::string& name);
        EntityID CreateEntityWithUUID(u64 uuid);

        void DuplicateEntity(EntityID entity);
        void CopyEntity(EntityID entity);

        void DestroyEntity(u64 uuid);

        EntityID GetEntity(const std::string& name);
        std::vector<EntityID> GetEntities();
        AssetManager& GetAssetManager();

    private:
        ECS* m_ECS;
        AssetManager* m_AssetManager;
        std::unordered_map<u64, EntityID> m_EntityMap;
        std::unordered_map<std::string, u64> m_NamedEntityMap;

        friend class Entity;
    };

} // namespace Blackberry
