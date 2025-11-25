#pragma once

#include "blackberry/ecs/components.hpp"
#include "blackberry/ecs/ecs.hpp"

namespace Blackberry {

    class PhysicsWorld {
    public:
        void SetContext(ECS& ecs);

        void Step(f32 ts);
        
    private:
        const f32 m_Gravity = 9.8f;
        ECS* m_Context = nullptr;
    };

} // namespace Blackberry