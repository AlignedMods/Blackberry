#include "blackberry/scene/scene.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/lua/lua.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/project/project.hpp"
#include "blackberry/scene/scene_renderer.hpp"
#include "blackberry/scene/scene_serializer.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

namespace Blackberry {

    Scene::Scene()
        : m_ECS(new ECS), m_PhysicsWorld(new PhysicsEngine), m_Renderer(new SceneRenderer(this)) {
        BL_CORE_TRACE("New scene created ({})", reinterpret_cast<void*>(this));
    }

    Scene::~Scene() {
        // Delete();
        // BL_CORE_TRACE("Scene destroyed ({})", reinterpret_cast<void*>(this));
    }

    Ref<Scene> Scene::Create(const FS::Path& path) {
        Ref<Scene> scene = CreateRef<Scene>();
        SceneSerializer serializer(scene);
        serializer.Deserialize(path);

        return scene;
    }

    void Scene::CopyTo(Ref<Scene> dest, Ref<Scene> source) {
        dest->m_ECS = ECS::Copy(source->m_ECS);

        dest->m_RootEntities = source->m_RootEntities;
        dest->m_EntityMap = source->m_EntityMap;
    }

    Ref<Scene> Scene::Copy(Ref<Scene> source) {
        Ref<Scene> scene = CreateRef<Scene>();

        CopyTo(scene, source);

        return scene;
    }

    void Scene::Delete() {}

    void Scene::OnRuntimeStart() {
        auto view = m_ECS->GetEntitiesWithComponents<ScriptComponent>();

        view.each([&](auto entity, ScriptComponent& script) {
            // Execute script
            Lua::RunFile(script.FilePath, script.ModulePath.String());
            Lua::SetExecutionContext(script.ModulePath.String());

            Lua::GetMember("OnAttach");
            Lua::CallFunction(0, 0);

            Lua::Pop(1);

            script.IsLoaded = true;
        });

        m_PhysicsWorld->SetContext(this);

        auto boxBodyView = m_ECS->GetEntitiesWithComponents<TransformComponent, RigidBodyComponent, BoxColliderComponent>();
        auto sphereBodyView = m_ECS->GetEntitiesWithComponents<TransformComponent, RigidBodyComponent, SphereColliderComponent>();

        boxBodyView.each([&](entt::entity entity, TransformComponent& transform, RigidBodyComponent& rigidbody, BoxColliderComponent& boxCollider) {
            m_PhysicsWorld->AddActor(static_cast<u32>(entity), transform, rigidbody, boxCollider);    
        });

        sphereBodyView.each([&](entt::entity entity, TransformComponent& transform, RigidBodyComponent& rigidbody, SphereColliderComponent& sphereCollider) {
            m_PhysicsWorld->AddActor(static_cast<u32>(entity), transform, rigidbody, sphereCollider);    
        });
    }

    void Scene::OnRuntimeStop() {
        auto view = m_ECS->GetEntitiesWithComponents<ScriptComponent>();

        view.each([&](auto entity, ScriptComponent& script) {
            if (script.IsLoaded) {
                Lua::SetExecutionContext(script.ModulePath.String());

                Lua::GetMember("OnDetach");
                Lua::CallFunction(0, 0);

                Lua::Pop(1);
            }
        });
    }

    SceneCamera Scene::GetSceneCamera() {
        SceneCamera cam;
        auto cameraView = m_ECS->GetEntitiesWithComponents<TransformComponent, CameraComponent>();

        u32 activeCameras = 0;

        cameraView.each([&](entt::entity entity, TransformComponent& transform, CameraComponent& camera) {
            if (camera.Active) {
                cam.Transform = transform;
                cam.Camera = camera;

                activeCameras++;
            }
        });

        if (activeCameras > 1) {
            BL_CORE_WARN("Multiple active cameras found in scene! Using the last one found.");
        }

        return cam;
    }

    void Scene::OnUpdateEditor() {}

    void Scene::OnUpdateRuntime() {
        if (m_Paused) return;

        auto scriptView = m_ECS->GetEntitiesWithComponents<ScriptComponent>();

        scriptView.each([&](auto entity, ScriptComponent& script) {
            Entity e(entity, this);

            Lua::SetExecutionContext(script.ModulePath.String());
            
            Lua::GetMember("OnUpdate");

            Lua::PushValue(-2); // push the table (self)
            Lua::PushNumber(BL_APP.GetDeltaTime());
            Lua::PushLightUserData(&e); // entity pointer

            Lua::CallFunction(3, 0);

            Lua::Pop(1);
        });

        m_PhysicsWorld->Step();
    }

    void Scene::OnRenderEditor(Ref<Framebuffer> target, SceneCamera& camera) {
        m_Renderer->SetCamera(camera);
        m_Renderer->SetRenderTarget(target);
        m_Renderer->Render(this);
    }

    void Scene::OnRenderRuntime(Ref<Framebuffer> target) {
        SceneCamera cam = GetSceneCamera();

        m_Renderer->SetCamera(cam);
        m_Renderer->SetRenderTarget(target);
        m_Renderer->Render(this);
    }

    EntityID Scene::CreateEntity(const std::string& name) {
        u64 id = UUID();
        CreateEntityWithUUID(id);
        m_NamedEntityMap[name] = id;

        TagComponent& tag = m_ECS->GetComponent<TagComponent>(m_EntityMap.at(id));
        tag.Name = name;
        
        return m_EntityMap.at(id);
    }

    EntityID Scene::CreateEntityWithUUID(u64 uuid) {
        m_EntityMap[uuid] = m_ECS->CreateEntity();

        m_ECS->AddComponent<TagComponent>(m_EntityMap.at(uuid), { "", uuid });
        m_ECS->AddComponent<RelationshipComponent>(m_EntityMap.at(uuid), {});

        return m_EntityMap.at(uuid);
    }

    void Scene::SetEntityParent(u64 entity, u64 parent) {
        DetachEntity(entity);

        RelationshipComponent& rel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(entity));
        rel.Parent = parent;

        if (parent != 0) {
            auto& pRel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(parent));

            u64 oldFirst = pRel.FirstChild;
            pRel.FirstChild = entity;

            rel.NextSibling = oldFirst;
            rel.PrevSibling = 0;

            if (oldFirst != 0) {
                auto& fRel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(oldFirst));
                fRel.PrevSibling = entity;
            }
        }
    }

    void Scene::DetachEntity(u64 uuid) {
        auto& rel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(uuid));

        if (rel.Parent != 0) {
            auto& pRel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(rel.Parent));

            if (pRel.FirstChild == uuid) {
                pRel.FirstChild = rel.NextSibling;
            }
        }

        if (rel.PrevSibling != 0) {
            auto& psRel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(rel.PrevSibling));

            psRel.NextSibling = rel.NextSibling;
        }

        if (rel.NextSibling != 0) {
            auto& nsRel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(rel.NextSibling));

            nsRel.PrevSibling = rel.PrevSibling;
        }

        rel.Parent = 0;
        rel.PrevSibling = 0;
        rel.NextSibling = 0;
    }

    void Scene::FinishEntityEdit(u64 entity) {
        auto& rel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(entity));

        std::erase(m_RootEntities, entity); // remove entity from root entities (because entity was probably parented to something)

        if (rel.Parent == 0) {
            m_RootEntities.push_back(entity); // if entity was not parented to something
        }
    }

    void Scene::DuplicateEntity(u64 entity) {
        auto newEntity = ECS::CopyEntity(m_EntityMap.at(entity), &m_ECS->m_Registry, &m_ECS->m_Registry);

        u64 uuid = m_ECS->GetComponent<TagComponent>(newEntity).UUID;
        m_EntityMap[uuid] = newEntity;

        FinishEntityEdit(uuid);
    }

    void Scene::DestroyEntity(u64 uuid) {
        BL_ASSERT(m_EntityMap.contains(uuid), "Entity with UUID {} does not exist!", uuid);

        auto& rel = m_ECS->GetComponent<RelationshipComponent>(m_EntityMap.at(uuid));
        if (rel.Parent == 0) {
            for (auto entity : m_RootEntities) {
                BL_CORE_INFO("vector before erase: {}", entity);
            }
            std::erase(m_RootEntities, uuid);
            for (auto entity : m_RootEntities) {
                BL_CORE_INFO("vector after erase: {}", entity);
            }
        }

        u64 child = rel.FirstChild;
        while (child != 0) {
            Entity childEntity = Entity(GetEntityFromUUID(child), this);
            RelationshipComponent childRel = childEntity.GetComponent<RelationshipComponent>(); // NOTE: **MAKE** A COPY TO THIS, THIS IS A COMPONENT OF A SOON TO BE DELETED ENTITY!
        
            DestroyEntity(child);
        
            child = childRel.NextSibling;
        }

        m_ECS->DestroyEntity(m_EntityMap.at(uuid));
        m_EntityMap.erase(uuid);
    }

    void Scene::SetPaused(bool pause) {
        m_Paused = pause;
    }

    bool Scene::IsPaused() const {
        return m_Paused;
    }

    EntityID Scene::GetEntity(const std::string& name) {
        if (!m_NamedEntityMap.contains(name)) {
            CreateEntity(name);
        }

        return m_EntityMap.at(m_NamedEntityMap.at(name));
    }

    EntityID Scene::GetEntityFromUUID(u64 uuid) {
        auto& entityMap = m_EntityMap;

        return entityMap.at(uuid);
    }

    std::vector<EntityID> Scene::GetEntities() {
        return m_ECS->GetAllEntities();
    }

    TransformComponent Scene::GetEntityParentTransform(EntityID e) {
        TransformComponent transform;

        RelationshipComponent rel = m_ECS->GetComponent<RelationshipComponent>(e);

        // We loop over all of the parents and add their transforms
        while (rel.Parent) {
            Entity parent(GetEntityFromUUID(rel.Parent), this);

            if (parent.HasComponent<TransformComponent>()) {
                auto& parentTransform = parent.GetComponent<TransformComponent>();
                transform.Position += parentTransform.Position;
            }

            rel = parent.GetComponent<RelationshipComponent>();
        }

        return transform;
    }

    TransformComponent Scene::GetEntityTransform(EntityID e) {
        BL_ASSERT(m_ECS->HasComponent<TransformComponent>(e), "Entity does not contain transform!");

        TransformComponent transform = m_ECS->GetComponent<TransformComponent>(e);
        transform.Position += GetEntityParentTransform(e).Position;

        return transform;
    }

    ECS* Scene::GetECS() {
        return m_ECS;
    }

    SceneRenderer* Scene::GetSceneRenderer() {
        return m_Renderer;
    }

    std::vector<u64>& Scene::GetRootEntities() {
        return m_RootEntities;
    }

} // namespace Blackberry
