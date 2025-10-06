#include "blackberry/ecs/ecs.hpp"
#include "blackberry/ecs/components.hpp"
#include "blackberry/rendering/rendering.hpp"

namespace Blackberry {

#pragma region RenderSystem

    void RenderSystem::Render() {
        using namespace Components;

        for (EntityID entity : m_Coordinator->GetEntities()) {
            if (m_Coordinator->HasComponent<Drawable>(entity) && m_Coordinator->HasComponent<Transform>(entity)) {
                Transform& trans = m_Coordinator->GetComponent<Transform>(entity);
                BlColor color = m_Coordinator->GetComponent<Drawable>(entity).Color;

                if (m_Coordinator->HasComponent<Material>(entity)) {
                    Material& material = m_Coordinator->GetComponent<Material>(entity);
                    BlRec area = (material.Area.w != 0.0f && material.Area.h != 0.0f) ? material.Area : BlRec(0.0f, 0.0f, static_cast<f32>(GetTextureWidth(material.Texture)), static_cast<f32>(GetTextureHeight(material.Texture)));
                    Blackberry::DrawTextureArea(trans.Position, trans.Dimensions, area, material.Texture);
                } else {
                    Blackberry::DrawRectangle(trans.Position, trans.Dimensions, color);
                }
            }
        }
    }

#pragma endregion

} // namespace Blackberry