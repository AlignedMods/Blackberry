#include "blackberry/scene/physics_world.hpp"

namespace Blackberry {

    void PhysicsWorld::AddEntity(const BlPhysicsEntity& entity) {
        m_Entities.push_back(entity);
    }

    void PhysicsWorld::Reset() {
        m_Entities.clear();
    }

    void PhysicsWorld::Step(f32 ts) {
        for (auto& entity : m_Entities) {
            entity.RigidBody->Force -= BlVec2(0.0f, m_Gravity * entity.RigidBody->Mass);

            entity.RigidBody->Acceleration = entity.RigidBody->Force * BlVec2(entity.RigidBody->Mass);

            entity.RigidBody->Velocity += entity.RigidBody->Acceleration * BlVec2(ts);
            entity.Transform->Position.x += entity.RigidBody->Velocity.x * ts;
            entity.Transform->Position.y += entity.RigidBody->Velocity.y * ts;

            entity.RigidBody->Force = BlVec2(0.0f);
        }
    }

} // namespace Blackberry