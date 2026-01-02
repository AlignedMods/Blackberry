#include "blackberry/physics/physics_engine.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/core/memory.hpp"
#include "blackberry/scene/scene.hpp"
#include "blackberry/scene/entity.hpp"

#include "Jolt/Jolt.h"
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

JPH_SUPPRESS_WARNINGS // Disable Jolt warnings

namespace Blackberry {

    // Physics layers
    // NOTE: we probably want to allow to customize the layers in the future
    namespace PhysicsLayers {

        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2;

    } // namespace PhysicsLayers

    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
            switch (inObject1) {
                case PhysicsLayers::NON_MOVING: return inObject2 == PhysicsLayers::NON_MOVING;
                case PhysicsLayers::MOVING: return true;
                default: BL_ASSERT(false, "Unreachable"); return false;
            }
        }
    };

    // Broad phase layers
    namespace PhysicsBroadPhaseLayers {
    
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::uint NUM_LAYERS(2);

    } // namespace PhysicsBroadPhaseLayers

    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        BPLayerInterfaceImpl() {
            m_ObjectToBroadPhase[PhysicsLayers::NON_MOVING] = PhysicsBroadPhaseLayers::NON_MOVING;
            m_ObjectToBroadPhase[PhysicsLayers::MOVING] = PhysicsBroadPhaseLayers::MOVING;
        }

        virtual JPH::uint GetNumBroadPhaseLayers() const override {
            return PhysicsBroadPhaseLayers::NUM_LAYERS;
        }

        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
            BL_ASSERT(inLayer < PhysicsLayers::NUM_LAYERS, "Out of range physics layer");
            return m_ObjectToBroadPhase[inLayer];
        }

    private:
        JPH::BroadPhaseLayer m_ObjectToBroadPhase[PhysicsLayers::NUM_LAYERS]{};
    };

    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
            switch (inLayer1) {
                case PhysicsLayers::NON_MOVING: return inLayer2 == PhysicsBroadPhaseLayers::MOVING;
                case PhysicsLayers::MOVING: return true;
                default: BL_ASSERT(false, "Unrachable"); return false;
            }
        }
    };

    class ContactListener : public JPH::ContactListener {
    public:
        virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override {
            BL_CORE_TRACE("Contact was added!");
        }

        virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override {
            BL_CORE_TRACE("Contact persisted!");
        }

        virtual void OnContactRemoved(const JPH::SubShapeIDPair& pair) override {
            BL_CORE_TRACE("Contact removed!");
        }
    };

    static JPH::TempAllocatorImpl* s_TempAllocator;
    static JPH::JobSystemThreadPool* s_JobSystem;
    static BPLayerInterfaceImpl s_BroadPhaseLayerInterface;
    static ObjectVsBroadPhaseLayerFilterImpl s_ObjectVsBroadPhaseLayerFilter;
    static ObjectLayerPairFilterImpl s_ObjectVsObjectLayerFilter;
    static ContactListener s_ContactListener;

    static constexpr JPH::uint MAX_BODIES = 65536;
    static constexpr JPH::uint MAX_BODY_MUTEXES = 0;
    static constexpr JPH::uint MAX_BODY_PAIRS = 65536;
    static constexpr JPH::uint MAX_CONTACT_CONSTRAINTS = 10240;

    static constexpr f32 DELTA_TIME = 1.0f / 60.0f;

    void PhysicsEngine::Initialize() {
        JPH::RegisterDefaultAllocator();

        s_TempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
        s_JobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

        JPH::Factory::sInstance = new JPH::Factory(); // toilet paper factory

        JPH::RegisterTypes();
    }

    void PhysicsEngine::Shutdown() {
        JPH::UnregisterTypes();

        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        delete s_TempAllocator;
        delete s_JobSystem;
    }

    PhysicsEngine::PhysicsEngine() {
        m_System = new JPH::PhysicsSystem;
        m_System->Init(MAX_BODIES, MAX_BODY_MUTEXES, MAX_BODY_PAIRS, MAX_CONTACT_CONSTRAINTS, s_BroadPhaseLayerInterface, s_ObjectVsBroadPhaseLayerFilter, s_ObjectVsObjectLayerFilter);

        m_System->SetContactListener(&s_ContactListener);
    }

    PhysicsEngine::~PhysicsEngine() {
        delete m_System;
        m_System = nullptr;
    }

    u32 PhysicsEngine::AddActor(u32 entity, TransformComponent& transform, RigidBodyComponent& rigidBody, BoxColliderComponent& boxCollider) {
        JPH::BodyInterface& bodyInterface = m_System->GetBodyInterface();

        JPH::BoxShapeSettings shapeSettings(JPH::Vec3(transform.Scale.x * boxCollider.Scale.x, transform.Scale.y * boxCollider.Scale.y, transform.Scale.z * boxCollider.Scale.z));
        shapeSettings.SetEmbedded();

        JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
        JPH::ShapeRefC shape = shapeResult.Get();

        JPH::EMotionType type;
        JPH::ObjectLayer layer;

        switch (rigidBody.Type) {
            case RigidBodyType::Static: type = JPH::EMotionType::Static; layer = PhysicsLayers::NON_MOVING; break;
            case RigidBodyType::Dynamic: type = JPH::EMotionType::Dynamic; layer = PhysicsLayers::MOVING; break;
            case RigidBodyType::Kinematic: type = JPH::EMotionType::Kinematic; layer = PhysicsLayers::MOVING; break;
            default: BL_ASSERT(false, "Unreachable"); break;
        }

        JPH::BodyCreationSettings bodySettings(shape, JPH::Vec3(transform.Position.x, transform.Position.y, transform.Position.z), 
                                                      JPH::Quat(transform.Rotation.x, transform.Rotation.y, transform.Rotation.z, transform.Rotation.w), type, layer);

        // Create the actual rigid body
        JPH::Body* body = bodyInterface.CreateBody(bodySettings);
        bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);

        // Apply body settings
        bodyInterface.SetRestitution(body->GetID(), rigidBody.Resitution);
        bodyInterface.SetFriction(body->GetID(), rigidBody.Friction);

        bodyInterface.SetUserData(body->GetID(), static_cast<u64>(entity));

        JPH::BodyID id = body->GetID();

        m_Actors.push_back(body);

        return static_cast<u32>(id.GetIndex());
    }

    u32 PhysicsEngine::AddActor(u32 entity, TransformComponent& transform, RigidBodyComponent& rigidBody, SphereColliderComponent& sphereCollider) {
        JPH::BodyInterface& bodyInterface = m_System->GetBodyInterface();

        f32 axis = std::max(transform.Scale.x, std::max(transform.Scale.y, transform.Scale.z));

        JPH::SphereShapeSettings shapeSettings(axis * sphereCollider.Radius);
        shapeSettings.SetEmbedded();

        JPH::ShapeSettings::ShapeResult shapeResult = shapeSettings.Create();
        JPH::ShapeRefC shape = shapeResult.Get();

        JPH::EMotionType type;
        JPH::ObjectLayer layer;

        switch (rigidBody.Type) {
            case RigidBodyType::Static: type = JPH::EMotionType::Static; layer = PhysicsLayers::NON_MOVING; break;
            case RigidBodyType::Dynamic: type = JPH::EMotionType::Dynamic; layer = PhysicsLayers::MOVING; break;
            case RigidBodyType::Kinematic: type = JPH::EMotionType::Kinematic; layer = PhysicsLayers::MOVING; break;
            default: BL_ASSERT(false, "Unreachable"); break;
        }

        TransformComponent realTransform = reinterpret_cast<Scene*>(m_Scene)->GetEntityTransform(static_cast<EntityID>(entity));
        BlQuat rot = glm::normalize(realTransform.Rotation);

        JPH::BodyCreationSettings bodySettings(shape, JPH::Vec3(realTransform.Position.x, realTransform.Position.y, realTransform.Position.z), 
                                                      JPH::Quat(rot.w, rot.x, rot.y, rot.z), type, layer);

        // Create the actual rigid body
        JPH::Body* body = bodyInterface.CreateBody(bodySettings);
        bodyInterface.AddBody(body->GetID(), JPH::EActivation::Activate);

        // Apply body settings
        bodyInterface.SetRestitution(body->GetID(), rigidBody.Resitution);
        bodyInterface.SetFriction(body->GetID(), rigidBody.Friction);

        bodyInterface.SetUserData(body->GetID(), static_cast<u64>(entity));

        JPH::BodyID id = body->GetID();

        m_Actors.push_back(body);

        return static_cast<u32>(id.GetIndex());
    }

    void PhysicsEngine::Step() {
        auto& bodyInterface = m_System->GetBodyInterface();

        // Send udated transforms back
        for (auto actor : m_Actors) {
            JPH::Body* body = reinterpret_cast<JPH::Body*>(actor);

            auto pos = body->GetCenterOfMassPosition();
            auto rot = body->GetRotation();
            EntityID entityID = static_cast<EntityID>(body->GetUserData());   

            Scene* scene = reinterpret_cast<Scene*>(m_Scene);

            Entity e(entityID, scene);
            auto& transform = e.GetComponent<TransformComponent>();

            transform.Position = BlVec3(pos.GetX(), pos.GetY(), pos.GetZ()) - reinterpret_cast<Scene*>(m_Scene)->GetEntityParentTransform(entityID).Position;
            transform.Rotation = glm::normalize(BlQuat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ()));
        }

        m_System->Update(BL_APP.GetDeltaTime(), 1, s_TempAllocator, s_JobSystem);
    }

    void PhysicsEngine::SetContext(void* scene) {
        m_Scene = scene;
    }

} // namespace Blackberry