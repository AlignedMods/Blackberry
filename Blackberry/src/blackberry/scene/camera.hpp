#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/core/util.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Blackberry {

    enum class CameraType {
        Orthographic = 0,
        Perspective = 1
    };

    class SceneCamera {
    public:
        SceneCamera() = default;

        glm::mat4 GetCameraMatrix() {
            auto& renderer = BL_APP.GetRenderer();
            BlVec2 viewport = renderer.GetViewportSize();

            glm::mat4 projection(1.0f);

            if (Type == CameraType::Orthographic) {
                projection = glm::ortho(
                                        0.0f,                               // left
                                        Size.x,                             // right
                                        0.0f,                               // bottom
                                        Size.y,                             // top
                                        Near, Far                           // near-far
                );
            } else if (Type == CameraType::Perspective) {
                BL_ASSERT(0, "not implemented");
            }
            

            glm::mat4 view(1.0f);
            view = glm::translate(view, glm::vec3(glm::vec2(Offset.x, Offset.y), 0.0f));
            view = glm::scale(view, glm::vec3(Zoom, Zoom, 1.0f));
            view = glm::rotate(view, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            view = glm::translate(view, glm::vec3(-glm::vec2(Position.x, Position.y), 0.0f));

            glm::mat4 finalProjection = projection * view;

            return finalProjection;
        }

        f32* GetCameraMatrixFloat() {
            return &(GetCameraMatrix()[0].x); // debatable memory safety but glm does this under the hood (but value_ptr returns a const* which we don't want)
        }

        BlVec2 GetScreenPosToWorld(BlVec2 position) {
            // NDC
            glm::vec2 ndc;
            ndc.x =  2.0f * (position.x / Size.x) - 1.0f;
            ndc.y =  1.0f - 2.0f * (position.y / Size.y);

            // NDC -> pixel
            glm::vec4 clipPos(ndc, 0.0f, 1.0f);

            glm::mat4 invCam = GetCameraMatrix();
            invCam = glm::inverse(invCam);

            glm::vec4 worldPos = invCam * clipPos;

            return BlVec2(worldPos.x, worldPos.y);
        }

    public:
        BlVec2 Position;
        BlVec2 Offset;
        BlVec2 Size = BlVec2(1920.0f, 1080.0f);
        f32 Zoom = 1.0f;
        f32 Rotation = 0.0f;
        f32 Near = -1.0f;
        f32 Far = 1.0f;

        CameraType Type = CameraType::Orthographic;
    };

} // namespace Blackberry