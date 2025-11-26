#include "blackberry/scene/physics_world.hpp"
#include "blackberry/scene/entity.hpp"

namespace Blackberry {

    static void NormalizeAABB(BlVec3<f32>& min, BlVec3<f32>& max) {
        if (min.x > max.x) std::swap(min.x, max.x);
        if (min.y > max.y) std::swap(min.y, max.y);
        if (min.z > max.z) std::swap(min.z, max.z);
    }

    static bool CollisionBetweenColliders(TransformComponent& t1, TransformComponent& t2, ColliderComponent& c1, ColliderComponent& c2) {
        if (c1.Type == ColliderType::Cube && c2.Type == ColliderType::Cube) {
            BlVec3<f32> min1 = t1.Position - t1.Scale * 0.5f;
            BlVec3<f32> max1 = t1.Position + t1.Scale * 0.5f;
            // BlVec3<f32> min1 = t1.Position;
            // BlVec3<f32> max1 = t1.Position - t1.Scale;
            NormalizeAABB(min1, max1);

            BlVec3<f32> min2 = t2.Position - t2.Scale * 0.5f;
            BlVec3<f32> max2 = t2.Position + t2.Scale * 0.5f;
            // BlVec3<f32> min2 = t2.Position;
            // BlVec3<f32> max2 = t2.Position - t2.Scale;
            NormalizeAABB(min2, max2);

            return (max1.x >= min2.x && min1.x <= max2.x) &&
                   (max1.y >= min2.y && min1.y <= max2.y) &&
                   (max1.z >= min2.z && min1.z <= max2.z);
        }

        return false;
    }

    void PhysicsWorld::SetContext(ECS& ecs) {
        m_Context = &ecs;
    }

    void PhysicsWorld::Step(f32 ts) {
        auto cView = m_Context->GetEntitiesWithComponents<TransformComponent, RigidBodyComponent, ColliderComponent>();

        // NOTE: VERY ineffecient currently
        // MUST be optimized in the very near future
        cView.each([&](entt::entity e1, TransformComponent& t1, RigidBodyComponent& rb1, ColliderComponent& c1) {
            cView.each([&](entt::entity e2, TransformComponent& t2, RigidBodyComponent& rb2, ColliderComponent& c2) {
                if (e1 == e2) return;

                if (CollisionBetweenColliders(t1, t2, c1, c2)) {
                    BL_CORE_WARN("Collision happened!");
                    rb1.LinearVelocity.y = 0.0f;
                }
            });
        });

        auto rbView = m_Context->GetEntitiesWithComponents<TransformComponent, RigidBodyComponent>();

        rbView.each([&](TransformComponent& transform, RigidBodyComponent& rb) {
            if (rb.Type == RigidBodyType::Dynamic) { // only apply forces if it's a dynamic body
                // Impulse forces
                rb.LinearVelocity += rb.ImpulseAccumulator / BlVec3(rb.Mass);

                // Add gravity
                // F = mg, aka force = mass * g (gravitational constant)
                rb.AddForce(BlVec3(0.0f, -(m_Gravity * rb.Mass), 0.0f));

                // a = F / m, aka acceleration = force / mass (you may realize that this completely cancels out gravity, but gravity may not be the only force so
                // we keep doing it this way)
                BlVec3<f32> acceleration = rb.ForceAccumulator / BlVec3(rb.Mass);

                rb.LinearVelocity += acceleration * BlVec3(ts);

                // Update transform
                transform.Position += rb.LinearVelocity * ts;
                transform.Rotation += rb.AngularVelocity * ts;

                // Reset force accumulators
                rb.ForceAccumulator = BlVec3(0.0f);
                rb.ImpulseAccumulator = BlVec3(0.0f);
            }
        });
    }

} // namespace Blackberry