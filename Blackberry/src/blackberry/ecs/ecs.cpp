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
                    BlRec area = (material.Area.w != 0.0f && material.Area.h != 0.0f) ? material.Area : BlRec(0.0f, 0.0f, static_cast<f32>(material.Texture.Width), static_cast<f32>(material.Texture.Height));
                    Blackberry::DrawTextureArea(trans.Position, trans.Dimensions, area, material.Texture, color);
                } else {
                    Blackberry::DrawRectangle(trans.Position, trans.Dimensions, color);
                }

                if (m_Coordinator->HasComponent<Text>(entity)) {
                    Text& text = m_Coordinator->GetComponent<Text>(entity);

                    Blackberry::DrawText(text.Contents, trans.Position, *text.Font, text.FontSize);
                }
            }
        }
    }

#pragma endregion

#pragma region PhysicsSystem

    void PhysicsSystem::RuntimeUpdate(f32 ts) {
        using namespace Components;

        for (EntityID entity : m_Coordinator->GetEntities()) {
            if (m_Coordinator->HasComponent<Transform>(entity) && m_Coordinator->HasComponent<Velocity>(entity)) {
                Transform& transform = m_Coordinator->GetComponent<Transform>(entity);

                transform.Position.x += m_Coordinator->GetComponent<Velocity>(entity).Acceleration.x * ts;
                transform.Position.y += m_Coordinator->GetComponent<Velocity>(entity).Acceleration.y * ts;
            }
        }
    }

#pragma endregion

} // namespace Blackberry