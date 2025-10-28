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
        : m_ECS(new ECS) {
        BL_CORE_TRACE("New scene created ({})", reinterpret_cast<void*>(this));
    }

    Scene::~Scene() {
        // Delete();
        BL_CORE_TRACE("Scene destroyed ({})", reinterpret_cast<void*>(this));
    }

    Scene* Scene::Copy(Scene* current) {
        Scene* scene = new Scene();

        scene->m_ECS = ECS::Copy(current->m_ECS);

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

        auto view = m_ECS->GetEntitiesWithComponents<Transform, Drawable>();

        view.each([&](auto entity, Transform& transform, Drawable& drawable) {
            RenderEntity(entity, transform, drawable);
        });

        Renderer2D::Render();
    }

    void Scene::RenderEntity(EntityID entity, Blackberry::Components::Transform& transform, Blackberry::Components::Drawable& drawable) {
        if (m_ECS->HasComponent<Material>(entity)) {
            Material& material = m_ECS->GetComponent<Material>(entity);

            Renderer2D::DrawTextureArea(transform.Position, transform.Dimensions, material.Area, material.Texture, transform.Rotation, drawable.Color);
        } else {
            switch (drawable.ShapeType) {
                case Shape::Triangle:
                    Renderer2D::DrawTriangle(transform.Position, transform.Dimensions, transform.Rotation, drawable.Color);
                    break;
                case Shape::Rectangle:
                    Renderer2D::DrawRectangle(transform.Position, transform.Dimensions, transform.Rotation, drawable.Color);
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

} // namespace Blackberry
