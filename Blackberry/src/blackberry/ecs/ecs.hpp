#pragma once

#include "blackberry/types.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/ecs/components.hpp"
#include "blackberry/util.hpp"

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <string>
#include <type_traits>

namespace Blackberry {

    using EntityID = u32;

    // we need to have some sort of generic component array
    class __ComponentArray {
    public:
        virtual void OnEntityDestroyed(EntityID entity) {}
    };

    template<typename T>
    class ComponentArray : public __ComponentArray {
    public:
        void AddComponent(EntityID entity, const T& component) {
            m_Components[entity] = component;
        }

        T& GetComponent(EntityID entity) {
            BL_ASSERT(m_Components.contains(entity), "Trying to get component that wanted entity doesn't contain!");
            return m_Components.at(entity);
        }

        bool HasComponent(EntityID entity) {
            return m_Components.contains(entity);
        }

        void RemoveComponent(EntityID entity) {
            BL_ASSERT(m_Components.contains(entity), "Trying to remove component that wanted entity doesn't contain!");
            m_Components.erase(entity);
        }

        virtual void OnEntityDestroyed(EntityID entity) {
            if (!m_Components.contains(entity)) { return; }

            m_Components.erase(entity);
        }

    private:
        std::unordered_map<EntityID, T> m_Components;
    };

    class ComponentManager {
    public:
        template<typename T>
        void RegisterComponent() {
            const char* type = typeid(T).name();

            m_ComponentArrays[type] = std::make_shared<ComponentArray<T>>();
        }

        template<typename T>
        void AddComponent(EntityID entity, const T& component) {
            auto comp = GetCompArray<T>();
            comp->AddComponent(entity, component);
        }

        template<typename T>
        T& GetComponent(EntityID entity) {
            auto comp = GetCompArray<T>();
            return comp->GetComponent(entity);
        }

        template<typename T>
        bool HasComponent(EntityID entity) {
            auto comp = GetCompArray<T>();
            return comp->HasComponent(entity);
        }

        template<typename T>
        void RemoveComponent(EntityID entity) {
            auto comp = GetCompArray<T>();
            comp->RemoveComponent(entity);
        }

        void OnEntityDestroyed(EntityID entity) {
            for (auto&[_, componentArray] : m_ComponentArrays) {
                componentArray->OnEntityDestroyed(entity);
            }
        }

    private:
        template<typename T>
        std::shared_ptr<ComponentArray<T>> GetCompArray() {
            return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays.at(typeid(T).name()));
        }

    private:
        std::unordered_map<const char*, std::shared_ptr<__ComponentArray>> m_ComponentArrays;
    };

    class __System {
    public:
        virtual void Update(f32 ts) {}
        virtual void Render() {}
    public:
        Coordinator* m_Coordinator = nullptr;
    };

    class PhysicsSystem : public __System {
    public:
        virtual void Update(f32 ts) override {};
    };

    class RenderSystem : public __System {
    public:
        virtual void Render() override;
    };

    class SystemManager {
    public:
        SystemManager() {
            m_Coordinator = nullptr;
        }

        SystemManager(Coordinator* coordinator) {
            m_Coordinator = coordinator;
        }

        template<typename T>
        void RegisterSystem() {
            const char* type = typeid(T).name();
            m_Systems[type] = std::make_shared<T>();
            m_Systems.at(type)->m_Coordinator = m_Coordinator;
        }

        void UpdateAllSystems() {
            for (auto&[_, system] : m_Systems) {
                system->Update(BL_APP.GetDeltaTime());
            }
        }

        void RenderAllSystems() {
            for (auto&[_, system] : m_Systems) {
                system->Render();
            }
        }

    private:
        std::unordered_map<const char*, std::shared_ptr<__System>> m_Systems;
        Coordinator* m_Coordinator = nullptr;
    };

    class Coordinator {
    public:
        Coordinator() {
            using namespace Components;

            m_ComponentManager = std::make_unique<ComponentManager>();
            m_SystemManager = std::make_unique<SystemManager>(this);

            // systems
            RegisterSystem<RenderSystem>();

            // components
            RegisterComponent<Drawable>();
            RegisterComponent<Transform>();
            RegisterComponent<Material>();

            m_Entities.reserve(200);
        }

        EntityID CreateEntity() {
            m_Entities.push_back(m_CurrentEntityID);
            EntityID id = m_CurrentEntityID;
            m_CurrentEntityID++;
            return id;
        }

        std::vector<EntityID>& GetEntities() {
            return m_Entities;
        }

        template<typename T>
        void RegisterSystem() {
            m_SystemManager->RegisterSystem<T>();
        }

        template<typename T>
        void RegisterComponent() {
            m_ComponentManager->RegisterComponent<T>();
        }

        template<typename T>
        void AddComponent(EntityID entity, const T& component = T{}) {
            m_ComponentManager->AddComponent<T>(entity, component);
        }

        template<typename T>
        bool HasComponent(EntityID entity) {
            return m_ComponentManager->HasComponent<T>(entity);
        }

        template<typename T>
        T& GetComponent(EntityID entity) {
            return m_ComponentManager->GetComponent<T>(entity);
        }

        void Update() {
            m_SystemManager->UpdateAllSystems();
        }

        void Render() {
            m_SystemManager->RenderAllSystems();
        }

    private:
        std::unique_ptr<ComponentManager> m_ComponentManager;
        std::unique_ptr<SystemManager> m_SystemManager;

        EntityID m_CurrentEntityID = 0;
        std::vector<EntityID> m_Entities;
    };

} // namespace Blackberry
