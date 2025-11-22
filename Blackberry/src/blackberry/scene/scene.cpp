#include "blackberry/scene/scene.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/renderer/renderer2d.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/lua/lua.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/project/project.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

namespace Blackberry {

    Scene::Scene()
        : m_ECS(new ECS), m_PhysicsWorld(new PhysicsWorld) {
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
            Lua::RunFile(script.FilePath, script.ModulePath.string());
            Lua::SetExecutionContext(script.ModulePath.string());

            Lua::GetMember("OnAttach");
            Lua::CallFunction(0, 0);

            Lua::Pop(1);

            script.IsLoaded = true;
        });
    }

    void Scene::OnStop() {
        auto view = m_ECS->GetEntitiesWithComponents<ScriptComponent>();

        view.each([&](auto entity, ScriptComponent& script) {
            if (script.IsLoaded) {
                Lua::SetExecutionContext(script.ModulePath.string());

                Lua::GetMember("OnDetach");
                Lua::CallFunction(0, 0);

                Lua::Pop(1);
            }
        });
    }

    SceneCamera Scene::GetSceneCamera() {
        SceneCamera cam;
        auto cameraView = m_ECS->GetEntitiesWithComponents<Transform2DComponent, CameraComponent>();

        cameraView.each([&](entt::entity entity, Transform2DComponent& transform, CameraComponent& camera) {
            if (camera.Active) {
                cam.Transform = transform;
                cam.Camera = camera;
            }
        });

        return cam;
    }

    void Scene::SetCamera(SceneCamera* camera) {
        m_Camera = camera;
    }

    void Scene::OnUpdate() {
        
    }

    void Scene::OnRuntimeUpdate() {
        auto scriptView = m_ECS->GetEntitiesWithComponents<ScriptComponent>();

        scriptView.each([&](auto entity, ScriptComponent& script) {
            Entity e(entity, this);

            Lua::SetExecutionContext(script.ModulePath.string());
            
            Lua::GetMember("OnUpdate");

            Lua::PushValue(-2); // push the table (self)
            Lua::PushNumber(BL_APP.GetDeltaTime());
            Lua::PushLightUserData(&e); // entity pointer

            Lua::CallFunction(3, 0);

            Lua::Pop(1);
        });

        auto rigidBodyView = m_ECS->GetEntitiesWithComponents<Transform2DComponent, RigidBodyComponent>();
        
        rigidBodyView.each([this](entt::entity entity, Transform2DComponent& transform, RigidBodyComponent& rigidBody) {
            m_PhysicsWorld->AddEntity({&transform, &rigidBody, nullptr});
        });

        m_PhysicsWorld->Step(BL_APP.GetDeltaTime());

        m_PhysicsWorld->Reset();
    }

    void Scene::OnRender() {
        BL_ASSERT(m_Camera, "No camera set for current scene!");

        Renderer2D::SetProjection(*m_Camera);

        // Render
        auto view = m_ECS->GetEntitiesWithComponents<Transform2DComponent>();

        view.each([&](auto entity, Transform2DComponent& transform) {
            RenderEntity(entity);
        });

        Renderer2D::Render();
        Renderer2D::ResetProjection();
    }

    void Scene::RenderEntity(EntityID entity) {
        Transform2DComponent& transform = m_ECS->GetComponent<Transform2DComponent>(entity);
        AssetManager& assetManager = Project::GetAssetManager();

        if (m_ECS->HasComponent<SpriteRendererComponent>(entity)) {
            SpriteRendererComponent& spriteRenderer = m_ECS->GetComponent<SpriteRendererComponent>(entity);

            if (spriteRenderer.TextureHandle > 0 && assetManager.ContainsAsset(spriteRenderer.TextureHandle)) {
                Asset asset = assetManager.GetAsset(spriteRenderer.TextureHandle);
                Texture2D tex = std::get<Texture2D>(asset.Data);

                switch (spriteRenderer.Shape) {
                    case ShapeType::Triangle:
                        Renderer2D::DrawTexturedTriangle(transform.GetMatrix(), spriteRenderer.Area, tex, spriteRenderer.Color);
                        break;
                    case ShapeType::Rectangle:
                        Renderer2D::DrawTexturedQuad(transform.GetMatrix(), spriteRenderer.Area, tex, spriteRenderer.Color);
                        break;
                }
            }
        }
        
        if (m_ECS->HasComponent<ShapeRendererComponent>(entity)) {
            ShapeRendererComponent& shapeRenderer = m_ECS->GetComponent<ShapeRendererComponent>(entity);

            switch (shapeRenderer.Shape) {
                case ShapeType::Triangle:
                    Renderer2D::DrawTriangle(transform.GetMatrix(), shapeRenderer.Color);
                    break;
                case ShapeType::Rectangle:
                    Renderer2D::DrawRectangle(transform.GetMatrix(), shapeRenderer.Color);
                    break;
                case ShapeType::Circle:
                    Renderer2D::DrawElipse(transform.GetMatrix(), shapeRenderer.Color);
                    break;
            }
        }

        if (m_ECS->HasComponent<TextComponent>(entity)) {
            TextComponent& text = m_ECS->GetComponent<TextComponent>(entity);

            if (assetManager.ContainsAsset(text.FontHandle)) {
                Asset asset = assetManager.GetAsset(text.FontHandle);
                Font& font = std::get<Font>(asset.Data);

                Renderer2D::DrawText(transform.GetMatrix(), text.Contents, font, {text.Kerning, text.LineSpacing});
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

    EntityID Scene::GetEntity(const std::string& name) {
        if (!m_NamedEntityMap.contains(name)) {
            CreateEntity(name);
        }

        return m_EntityMap.at(m_NamedEntityMap.at(name));
    }

    std::vector<EntityID> Scene::GetEntities() {
        return m_ECS->GetAllEntities();
    }

} // namespace Blackberry
