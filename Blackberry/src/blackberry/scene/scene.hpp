#pragma once

#include "blackberry/scene/uuid.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/physics/physics_engine.hpp"
#include "blackberry/scene/camera.hpp"

#include <unordered_map>
#include <string>

namespace Blackberry {

    class SceneRenderer;

    class Scene {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Create(const FS::Path& path);

        static void CopyTo(Ref<Scene> dest, Ref<Scene> source);
        static Ref<Scene> Copy(Ref<Scene> source);
        void Delete();

        void OnRuntimeStart();
        void OnRuntimeStop();

        SceneCamera GetSceneCamera();

        void OnUpdateEditor();
        void OnUpdateRuntime();

        void OnRenderEditor(Ref<Framebuffer> target, SceneCamera& camera);
        void OnRenderRuntime(Ref<Framebuffer> target);

        EntityID CreateEntity(const std::string& name);
        EntityID CreateEntityWithUUID(u64 uuid);

        void SetEntityParent(u64 entity, u64 parent);
        void DetachEntity(u64 uuid);

        void FinishEntityEdit(u64 entity);

        void DuplicateEntity(u64 entity);

        void DestroyEntity(u64 uuid);

        void SetPaused(bool pause);
        bool IsPaused() const;

        EntityID GetEntity(const std::string& name);
        EntityID GetEntityFromUUID(u64 uuid);
        std::vector<EntityID> GetEntities();

        // NOTE: This gets ALL of the parent's transforms, not just one
        TransformComponent GetEntityParentTransform(EntityID e);
        TransformComponent GetEntityTransform(EntityID e);

        ECS* GetECS();
        SceneRenderer* GetSceneRenderer();

        std::vector<u64>& GetRootEntities();

    private:
        ECS* m_ECS = nullptr;
        PhysicsEngine* m_PhysicsWorld = nullptr;
        SceneRenderer* m_Renderer = nullptr;
        std::unordered_map<u64, EntityID> m_EntityMap;
        std::vector<u64> m_RootEntities;
        std::unordered_map<std::string, u64> m_NamedEntityMap;

        const f32 m_Gravity = 9.8f;

        bool m_Paused = false;

        friend class Entity;
        friend class SceneRenderer;
    };

} // namespace Blackberry
