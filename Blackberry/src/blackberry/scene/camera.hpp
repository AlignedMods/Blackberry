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

            BlVec3 forward = Transform.Rotation * BlVec3(0, 0, -1);
            BlVec3 up = Transform.Rotation * BlVec3(0, 1, 0);

            view = glm::lookAt(Transform.Position, Transform.Position + forward, up);

            return view;
        }

        BlVec3 GetForwardVector() {
            return Transform.Rotation * BlVec3(0, 0, -1);
        }

        BlVec3 GetRightVector() {
            return Transform.Rotation * BlVec3(1, 0, 0);
        }

        BlVec3 GetUpVector() {
            return Transform.Rotation * BlVec3(0, 1, 0);
        }

        f32* GetCameraMatrixFloat() {
            return &(GetCameraMatrix()[0].x); // debatable memory safety but glm does this under the hood (but value_ptr returns a const* which we don't want)
        }

    public:
        // Components which get set externally
        TransformComponent Transform;
        CameraComponent Camera;
    };

} // namespace Blackberry