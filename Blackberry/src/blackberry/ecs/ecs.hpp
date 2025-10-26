#pragma once

#include "blackberry/ecs/components.hpp"
#include "blackberry/core/log.hpp"

#include "../vendor/entt/entt.hpp"

namespace Blackberry {

    using namespace Components;

    using EntityID = entt::entity;

    template <typename T>
    inline static void CopyComponent(entt::registry* dest, entt::registry* src, entt::entity destEntity, entt::entity srcEntity) {
        if (src->any_of<T>(srcEntity)) {
            BL_CORE_TRACE("Copying component {} of entity {}", typeid(T).name(), static_cast<u32>(srcEntity));
            dest->emplace<T>(destEntity, src->get<T>(srcEntity));
        }
    }

    class ECS {
    public:
        ECS() = default;
        ~ECS() = default;

        static ECS* Copy(ECS* current) {
            ECS* newECS = new ECS();

            current->m_Registry.view<entt::entity>().each([&](auto entity) {
                BL_CORE_INFO("Copying entity {}", static_cast<u32>(entity));
                const auto newEntity = newECS->m_Registry.create(entity);

                CopyComponent<Tag>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
                CopyComponent<Transform>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
                CopyComponent<Drawable>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
                CopyComponent<Material>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
                CopyComponent<Text>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
                CopyComponent<Script>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
                CopyComponent<Velocity>(&newECS->m_Registry, &current->m_Registry, newEntity, entity);
            });

            return newECS;
        }

        EntityID CreateEntity() {
            return m_Registry.create();
        }

        void DestroyEntity(EntityID entity) {
            m_Registry.destroy(entity);
        }

        template <typename T>
        void AddComponent(EntityID entity, const T& component) {
            m_Registry.emplace<T>(entity, component);
        }

        template <typename T>
        bool HasComponent(EntityID entity) {
            return m_Registry.any_of<T>(entity);
        }

        template <typename T>
        T& GetComponent(EntityID entity) {
            return m_Registry.get<T>(entity);
        }

        template <typename T>
        void RemoveComponent(EntityID entity) {
            m_Registry.remove<T>(entity);
        }

        std::vector<EntityID> GetAllEntities() {
            std::vector<EntityID> entities;
            m_Registry.view<entt::entity>().each([&](auto entity) {
                entities.push_back(entity);
            });
            return entities;
        }

        template <typename... T>
        auto GetEntitiesWithComponents() {
            return m_Registry.view<T...>();
        }

    private:
        entt::registry m_Registry;
    };

} // namespace Blackberry