#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/application/application.hpp"
#include "blackberry/core/util.hpp"
#include "blackberry/ecs/components.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Blackberry {

    class SceneCamera {
    public:
        SceneCamera() = default;

        glm::mat4 GetCameraMatrix() {
            glm::mat4 projection = GetCameraProjection();
            glm::mat4 view = GetCameraView();
            
            glm::mat4 finalProjection = projection * view;
            return finalProjection;
        }

        glm::mat4 GetCameraProjection() {
            glm::mat4 projection(1.0f);

            if (Camera.Type == CameraType::Orthographic) {
                projection = glm::ortho(
                                        0.0f,                               // left
                                        Transform.Scale.x,                  // right
                                        0.0f,                               // bottom
                                        Transform.Scale.y,                  // top
                                        Camera.Near, Camera.Far             // near-far
                );
            } else if (Camera.Type == CameraType::Perspective) {
                BL_ASSERT(false, "Not supported!");
            }

            return projection;
        }

        // NOTE: The view you get fron this function is already inversed!!
        glm::mat4 GetCameraView() {
            glm::mat4 view(1.0f);

            glm::mat4 offset = glm::translate(glm::mat4(1.0f), glm::vec3(glm::vec2(Transform.Scale.x * 0.5f, Transform.Scale.y * 0.5f), 0.0f));
            glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / Camera.Zoom, 1.0f / Camera.Zoom, 1.0f));
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(Transform.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            glm::mat4 pos = glm::translate(glm::mat4(1.0f), glm::vec3(-glm::vec2(Transform.Position.x + Transform.Scale.x * 0.5f, Transform.Position.y + Transform.Scale.y * 0.5f), 0.0f));

            view = offset * scale * rot * pos;

            return glm::inverse(view);
        }

        f32* GetCameraMatrixFloat() {
            return &(GetCameraMatrix()[0].x); // debatable memory safety but glm does this under the hood (but value_ptr returns a const* which we don't want)
        }

        BlVec2<f32> GetScreenPosToWorld(BlVec2<f32> position) {
            // NDC
            glm::vec2 ndc;
            ndc.x =  2.0f * (position.x / Transform.Scale.x) - 1.0f;
            ndc.y =  1.0f - 2.0f * (position.y / Transform.Scale.y);

            // NDC -> pixel
            glm::vec4 clipPos(ndc, 0.0f, 1.0f);

            glm::mat4 invCam = GetCameraMatrix();
            invCam = glm::inverse(invCam);

            glm::vec4 worldPos = invCam * clipPos;

            return BlVec2(worldPos.x, worldPos.y);
        }

    public:
        // Components which get set externally
        TransformComponent Transform;
        CameraComponent Camera;
    };

} // namespace Blackberry