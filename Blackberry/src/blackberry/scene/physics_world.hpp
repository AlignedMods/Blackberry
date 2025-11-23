#pragma once

#include "blackberry/ecs/components.hpp"

struct BlPhysicsEntity {
    Blackberry::TransformComponent* Transform = nullptr;
    Blackberry::RigidBodyComponent* RigidBody = nullptr;
    Blackberry::BoxColliderComponent* BoxCollider = nullptr;
};

namespace Blackberry {

    class PhysicsWorld {
    public:
        void AddEntity(const BlPhysicsEntity& entity);
        void Reset();

        void Step(f32 ts);
        
    private:
        const f32 m_Gravity = 9.8f;
        std::vector<BlPhysicsEntity> m_Entities;
    };

} // namespace Blackberry