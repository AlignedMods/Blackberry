#include "blackberry/scene/scene.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/renderer/renderer3d.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/lua/lua.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/project/project.hpp"
#include "blackberry/scene/scene_renderer.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

namespace Blackberry {

    Scene::Scene()
        : m_ECS(new ECS), m_PhysicsWorld(new PhysicsWorld), m_Renderer(new SceneRenderer) {
        BL_CORE_TRACE("New scene created ({})", reinterpret_cast<void*>(this));
    }

    Scene::~Scene() {
        // Delete();
        // BL_CORE_TRACE("Scene destroyed ({})", reinterpret_cast<void*>(this));
    }

    Scene* Scene::Copy(Scene* current) {
        Scene* scene = new Scene();

        scene->m_ECS = ECS::Copy(current->m_ECS);

        return scene;
    }

    void Scene::Delete() {}

    void Scene::OnPlay() {
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

        // set context for physics
        m_PhysicsWorld->SetContext(*m_ECS);
    }

    void Scene::OnStop() {
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

    void Scene::SetCamera(SceneCamera* camera) {
        m_Camera = camera;
        m_Renderer->SetCamera(*camera);
    }

    void Scene::OnUpdate() {
        
    }

    void Scene::OnRuntimeUpdate() {
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

        m_PhysicsWorld->Step(BL_APP.GetDeltaTime());
    }

    void Scene::OnRender(RenderTexture* target) {
        BL_ASSERT(m_Camera, "No camera set for current scene!");

        m_Renderer->Render(this, target);
    }

    void Scene::RenderEntity(EntityID entity) {
        TransformComponent& transform = m_ECS->GetComponent<TransformComponent>(entity);
        AssetManager& assetManager = Project::GetAssetManager();

        if (m_ECS->HasComponent<MeshComponent>(entity)) {
            MeshComponent& mesh = m_ECS->GetComponent<MeshComponent>(entity);

            if (assetManager.ContainsAsset(mesh.MeshHandle)) {
                Asset asset = assetManager.GetAsset(mesh.MeshHandle);
                Model& model = std::get<Model>(asset.Data);

                Renderer3D::DrawModel(transform.GetMatrix(), model, BlColor(155, 255, 100, 255));
            }
        }

        if (m_ECS->HasComponent<TextComponent>(entity)) {
            TextComponent& text = m_ECS->GetComponent<TextComponent>(entity);

            if (assetManager.ContainsAsset(text.FontHandle)) {
                Asset asset = assetManager.GetAsset(text.FontHandle);
                Font& font = std::get<Font>(asset.Data);

                Renderer3D::DrawText(transform.GetMatrix(), text.Contents, font, {text.Kerning, text.LineSpacing});
            }
        }
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

        return m_EntityMap.at(uuid);
    }

    void Scene::DuplicateEntity(EntityID entity) {
        ECS::DuplicateEntity(entity, &m_ECS->m_Registry, &m_ECS->m_Registry);
    }

    void Scene::CopyEntity(EntityID entity) {
        ECS::CopyEntity(entity, &m_ECS->m_Registry, &m_ECS->m_Registry);
    }

    void Scene::DestroyEntity(u64 uuid) {
        BL_ASSERT(m_EntityMap.contains(uuid), "Entity with UUID {} does not exist!", uuid);

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

    std::vector<EntityID> Scene::GetEntities() {
        return m_ECS->GetAllEntities();
    }

    SceneRenderer* Scene::GetSceneRenderer() {
        return m_Renderer;
    }

} // namespace Blackberry
