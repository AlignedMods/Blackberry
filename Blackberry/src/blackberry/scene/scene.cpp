#include "blackberry/scene/scene.hpp"
#include "blackberry/ecs/ecs.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/rendering/rendering.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/lua/lua.hpp"
#include "blackberry/scene/entity.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

namespace Blackberry {

    Scene::Scene()
        : m_ECS(new ECS), m_AssetManager(new AssetManager) {
        BL_CORE_TRACE("New scene created ({})", reinterpret_cast<void*>(this));
    }

    Scene::~Scene() {
        // Delete();
        BL_CORE_TRACE("Scene destroyed ({})", reinterpret_cast<void*>(this));
    }

    Scene* Scene::Copy(Scene* current) {
        Scene* scene = new Scene();

        scene->m_ECS = ECS::Copy(current->m_ECS);
        scene->m_AssetManager = AssetManager::Copy(current->m_AssetManager);

        return scene;
    }

    void Scene::Delete() {}

    void Scene::OnPlay() {
        auto view = m_ECS->GetEntitiesWithComponents<Script>();

        view.each([&](auto entity, Script& script) {
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
        auto view = m_ECS->GetEntitiesWithComponents<Script>();

        view.each([&](auto entity, Script& script) {
            if (script.IsLoaded) {
                Lua::SetExecutionContext(script.ModulePath.string());

                Lua::GetMember("OnDetach");
                Lua::CallFunction(0, 0);

                Lua::Pop(1);
            }
        });
    }

    void Scene::OnUpdate() {
        
    }

    void Scene::OnRuntimeUpdate() {
        using namespace Components;

        auto view = m_ECS->GetEntitiesWithComponents<Script>();

        view.each([&](auto entity, Script& script) {
            Entity e(entity, this);

            Lua::SetExecutionContext(script.ModulePath.string());
            
            Lua::GetMember("OnUpdate");

            Lua::PushValue(-2); // push the table (self)
            Lua::PushNumber(BL_APP.GetDeltaTime());
            Lua::PushLightUserData(&e); // entity pointer

            Lua::CallFunction(3, 0);

            Lua::Pop(1);
        });
    }

    void Scene::OnRender() {
        using namespace Components;

        // Render

        auto view = m_ECS->GetEntitiesWithComponents<Transform>();

        view.each([&](auto entity, Transform& transform) {
            RenderEntity(entity);
        });

        Renderer2D::Render();
    }

    void Scene::RenderEntity(EntityID entity) {
        using namespace Components;

        Transform& transform = m_ECS->GetComponent<Transform>(entity);

        if (m_ECS->HasComponent<SpriteRenderer>(entity)) {
            SpriteRenderer& spriteRenderer = m_ECS->GetComponent<SpriteRenderer>(entity);

            if (spriteRenderer.TextureHandle > 0 && m_AssetManager->ContainsAsset(spriteRenderer.TextureHandle)) {
                Asset asset = m_AssetManager->GetAsset(spriteRenderer.TextureHandle);
                BlTexture tex = std::get<BlTexture>(asset.Data);

                Renderer2D::DrawTextureArea(transform.Position, transform.Dimensions, spriteRenderer.Area, tex, transform.Rotation, spriteRenderer.Color);
            }
        }
        
        if (m_ECS->HasComponent<ShapeRenderer>(entity)) {
            ShapeRenderer& shapeRenderer = m_ECS->GetComponent<ShapeRenderer>(entity);

            switch (shapeRenderer.Shape) {
                case ShapeType::Triangle:
                    Renderer2D::DrawTriangle(transform.Position, transform.Dimensions, transform.Rotation, shapeRenderer.Color);
                    break;
                case ShapeType::Rectangle:
                    Renderer2D::DrawRectangle(transform.Position, transform.Dimensions, transform.Rotation, shapeRenderer.Color);
                    break;
            }
        }
    }

    EntityID Scene::CreateEntity(const std::string& name) {
        u64 id = UUID();
        CreateEntityWithUUID(id);
        m_NamedEntityMap[name] = id;

        Components::Tag& tag = m_ECS->GetComponent<Components::Tag>(m_EntityMap.at(id));
        tag.Name = name;
        
        return m_EntityMap.at(id);
    }

    EntityID Scene::CreateEntityWithUUID(u64 uuid) {
        m_EntityMap[uuid] = m_ECS->CreateEntity();

        m_ECS->AddComponent<Components::Tag>(m_EntityMap.at(uuid), { "", uuid });

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

    AssetManager& Scene::GetAssetManager() {
        return *m_AssetManager;
    }

} // namespace Blackberry
