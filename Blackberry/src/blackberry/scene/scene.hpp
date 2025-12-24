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

        static Scene* Copy(Scene* current);
        void Delete();

        void OnPlay();
        void OnStop();

        SceneCamera GetSceneCamera();
        void SetCamera(SceneCamera* camera);

        void OnUpdate();
        void OnRuntimeUpdate();
        void OnRender(RenderTexture* target = nullptr); // NOTE: you can use a custom framebuffer here!

        EntityID CreateEntity(const std::string& name);
        EntityID CreateEntityWithUUID(u64 uuid);

        void DuplicateEntity(EntityID entity);
        void CopyEntity(EntityID entity);

        void DestroyEntity(u64 uuid);

        void SetPaused(bool pause);
        bool IsPaused() const;

        EntityID GetEntity(const std::string& name);
        std::vector<EntityID> GetEntities();

        SceneRenderer* GetSceneRenderer();

    private:
        ECS* m_ECS = nullptr;
        PhysicsEngine* m_PhysicsWorld = nullptr;
        SceneCamera* m_Camera = nullptr; // gets set in OnRuntimeUpdate
        SceneRenderer* m_Renderer = nullptr;
        std::unordered_map<u64, EntityID> m_EntityMap;
        std::unordered_map<std::string, u64> m_NamedEntityMap;

        const f32 m_Gravity = 9.8f;

        bool m_Paused = false;

        friend class Entity;
        friend class SceneRenderer;
    };

} // namespace Blackberry
