#include "blackberry/scene/physics_world.hpp"
#include "blackberry/scene/entity.hpp"

namespace Blackberry {

    void PhysicsWorld::SetContext(ECS& ecs) {
        m_Context = &ecs;
    }

    void PhysicsWorld::Step(f32 ts) {
        auto view = m_Context->GetEntitiesWithComponents<TransformComponent, RigidBodyComponent>();

        view.each([&](TransformComponent& transform, RigidBodyComponent& rb) {
            if (rb.Mass == 0.0f) return; // not so fast (there are divisions by mass in here, aka 0 if mass is 0)

            // Add gravity
            // F = mg, aka force = mass * g (gravitational constant)
            rb.AddForce(BlVec3(0.0f, -(m_Gravity * rb.Mass), 0.0f));

            // a = F / m, aka acceleration = force / mass (you may realize that this completely cancels out gravity, but gravity may not be the only force so
            // we keep doing it this way)
            rb.Acceleration = rb.Force / BlVec3(rb.Mass);

            rb.Velocity += rb.Acceleration * BlVec3(ts);
            transform.Position += rb.Velocity * ts;

            rb.Force = BlVec3(0.0f);
        });
    }

} // namespace Blackberry