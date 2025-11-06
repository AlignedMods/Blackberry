#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/application/application.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Blackberry {

    class SceneCamera {
    public:
        SceneCamera() = default;

        glm::mat4 GetCameraMatrix() {
            auto& renderer = BL_APP.GetRenderer();
            BlVec2 viewport = renderer.GetViewportSize();

            glm::mat4 projection = glm::ortho(
                                    0.0f,                               // left
                                    static_cast<f32>(viewport.x),       // right
                                    static_cast<f32>(viewport.y),       // bottom
                                    0.0f,                               // top
                                    -1000.0f, 1000.0f                   // near-far
            );

            glm::mat4 view(1.0f);
            view = glm::translate(view, glm::vec3(glm::vec2(Offset.x, Offset.y), 0.0f));
            view = glm::scale(view, glm::vec3(Scale, Scale, 1.0f));
            view = glm::rotate(view, glm::radians(Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            view = glm::translate(view, glm::vec3(-glm::vec2(Position.x, Position.y), 0.0f));

            glm::mat4 finalProjection = projection * view;

            return finalProjection;
        }

        f32* GetCameraMatrixFloat() {
            return &(GetCameraMatrix()[0].x); // debatable memory safety but glm does this under the hood (but value_ptr returns a const* which we don't want)
        }

    public:
        BlVec2 Position;
        BlVec2 Offset;
        f32 Scale = 1.0f;
        f32 Rotation = 0.0f;
    };

} // namespace Blackberry