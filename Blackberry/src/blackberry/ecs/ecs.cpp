#include "blackberry/ecs/ecs.hpp"
#include "blackberry/ecs/components.hpp"
#include "blackberry/rendering/rendering.hpp"

namespace Blackberry {

    static __EntityID s_CurrentID;

    __EntityID CreateEntity() {
        return s_CurrentID++;
    }

#pragma region RenderSystem

    void RenderSystem::Render() {
        for (__EntityID entity : m_Coordinator->GetEntities()) {
            if (m_Coordinator->HasComponent<Drawable>(entity) && m_Coordinator->HasComponent<Transform>(entity)) {
                Transform& trans = m_Coordinator->GetComponent<Transform>(entity);
                BlColor color = (m_Coordinator->HasComponent<Color>(entity)) ? m_Coordinator->GetComponent<Color>(entity).Fill : BlColor();
                Blackberry::DrawRectangle(trans.Position, trans.Dimensions, color);
            }
        }
    }

#pragma endregion

} // namespace Blackberry