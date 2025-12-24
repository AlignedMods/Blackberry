#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/ecs/components.hpp"

namespace JPH {

    class PhysicsSystem;

} // namespace JPH

namespace Blackberry {
    
    class PhysicsEngine {
    public:
        static void Initialize();
        static void Shutdown();

        PhysicsEngine();
        ~PhysicsEngine();

        u32 AddActor(u32 entity, TransformComponent& transform, RigidBodyComponent& rigidBody, BoxColliderComponent& boxCollider);
        u32 AddActor(u32 entity, TransformComponent& transform, RigidBodyComponent& rigidBody, SphereColliderComponent& sphereCollider);

        void Step();
        void SetContext(void* scene);

    private:
        JPH::PhysicsSystem* m_System = nullptr;

        std::vector<void*> m_Actors;
        void* m_Scene = nullptr;
    };

}