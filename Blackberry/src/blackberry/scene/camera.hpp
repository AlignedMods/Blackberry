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

            projection = glm::perspective(
                                    glm::radians(Camera.FOV / Camera.Zoom),
                                    Transform.Scale.x / Transform.Scale.y,
                                    Camera.Near, Camera.Far
            );

            return projection;
        }

        // NOTE: The view you get fron this function is already inversed!!
        glm::mat4 GetCameraView() {
            glm::mat4 view(1.0f);

            glm::vec3 cameraPos   = glm::vec3(Transform.Position.x, Transform.Position.y, Transform.Position.z);

            // calculate front vector from rotation (euler angles in degrees)
            glm::vec3 cameraFront;
            cameraFront.x = glm::cos(glm::radians(Transform.Rotation.x)) * cos(glm::radians(Transform.Rotation.y));
            cameraFront.y = glm::sin(glm::radians(Transform.Rotation.y));
            cameraFront.z = glm::sin(glm::radians(Transform.Rotation.x)) * cos(glm::radians(Transform.Rotation.y));
            cameraFront = glm::normalize(cameraFront);

            view = glm::lookAt(cameraPos, cameraPos + cameraFront, WorldUp);

            return view;
        }

        BlVec3<f32> GetForwardVector() {
            glm::vec3 cameraFront;
            cameraFront.x = glm::cos(glm::radians(Transform.Rotation.x)) * cos(glm::radians(Transform.Rotation.y));
            cameraFront.y = glm::sin(glm::radians(Transform.Rotation.y));
            cameraFront.z = glm::sin(glm::radians(Transform.Rotation.x)) * cos(glm::radians(Transform.Rotation.y));
            cameraFront = glm::normalize(cameraFront);

            return BlVec3(cameraFront.x, cameraFront.y, cameraFront.z);
        }

        BlVec3<f32> GetRightVector() {
            glm::vec3 cameraFront;
            cameraFront.x = glm::cos(glm::radians(Transform.Rotation.x)) * cos(glm::radians(Transform.Rotation.y));
            cameraFront.y = glm::sin(glm::radians(Transform.Rotation.y));
            cameraFront.z = glm::sin(glm::radians(Transform.Rotation.x)) * cos(glm::radians(Transform.Rotation.y));
            cameraFront = glm::normalize(cameraFront);

            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, WorldUp));

            return BlVec3(cameraRight.x, cameraRight.y, cameraRight.z);
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

        glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    };

} // namespace Blackberry