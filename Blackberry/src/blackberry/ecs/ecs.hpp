#pragma once

#include "blackberry/ecs/components.hpp"
#include "blackberry/core/log.hpp"

#include "../vendor/entt/entt.hpp"

namespace Blackberry {

    using EntityID = entt::entity;

    template <typename T>
    inline static void CopyComponent(entt::registry* src, entt::registry* dest, entt::entity srcEntity, entt::entity destEntity) {
        if (src->any_of<T>(srcEntity)) {
            BL_CORE_TRACE("Copying component {} of entity {}", typeid(T).name(), static_cast<u32>(srcEntity));
            dest->emplace<T>(destEntity, src->get<T>(srcEntity));
        }
    }

    class ECS {
    public:
        ECS() = default;
        ~ECS() = default;

        // literally copies entity (NOTE: does NOT create new UUIDs!!)
        // be VERY careful using this!
        static void DuplicateEntity(entt::entity target, entt::registry* srcReg, entt::registry* destReg) {
            BL_CORE_INFO("DUPLICATING entity {}", static_cast<u32>(target));

            entt::registry* src = srcReg;
            entt::registry* dest = destReg;

            const auto newEntity = dest->create(target);

            CopyComponent<TagComponent>(src, dest, target, newEntity);
            CopyComponent<TransformComponent>(src, dest, target, newEntity);
            CopyComponent<ShapeRendererComponent>(src, dest, target, newEntity);
            CopyComponent<SpriteRendererComponent>(src, dest, target, newEntity);
            CopyComponent<TextComponent>(src, dest, target, newEntity);
            CopyComponent<ScriptComponent>(src, dest, target, newEntity);
            CopyComponent<VelocityComponent>(src, dest, target, newEntity);
        }

        // safer version on duplicate entity (generates new UUIDs)
        static void CopyEntity(entt::entity target, entt::registry* srcReg, entt::registry* destReg) {
            BL_CORE_INFO("COPYING entity {}", static_cast<u32>(target));

            entt::registry* src = srcReg;
            entt::registry* dest = destReg;

            const auto newEntity = dest->create(target);
            TagComponent tag;
            tag.Name = src->get<TagComponent>(target).Name;
            tag.UUID = UUID();

            dest->emplace<TagComponent>(newEntity, tag);

            CopyComponent<TransformComponent>(src, dest, target, newEntity);
            CopyComponent<ShapeRendererComponent>(src, dest, target, newEntity);
            CopyComponent<SpriteRendererComponent>(src, dest, target, newEntity);
            CopyComponent<TextComponent>(src, dest, target, newEntity);
            CopyComponent<ScriptComponent>(src, dest, target, newEntity);
            CopyComponent<VelocityComponent>(src, dest, target, newEntity);
        }

        static ECS* Copy(ECS* current) {
            ECS* newECS = new ECS();

            auto view = current->m_Registry.view<entt::entity>();

            for (auto it = view.rbegin(); it < view.rend(); it++) {
                auto entity = *it;
                DuplicateEntity(entity, &current->m_Registry, &newECS->m_Registry);
            }

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

        friend class Scene;
    };

} // namespace Blackberry