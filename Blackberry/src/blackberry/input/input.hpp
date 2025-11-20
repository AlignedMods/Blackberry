#pragma once

#include "blackberry/input/keycodes.hpp"
#include "blackberry/input/mousebuttons.hpp"

namespace Blackberry {

    class Input {
    public:
        static bool IsKeyDown(KeyCode key);
        static bool IsKeyPressed(KeyCode key);

        static bool IsMouseDown(MouseButton key);
        static bool IsMousePressed(MouseButton key);
        static bool IsMouseReleased(MouseButton key);

        static BlVec2<f32> GetMousePosition();
        static BlVec2<f32> GetMouseDelta();
        static f32 GetScrollLevel();

        static void SetKeyState(KeyCode key, bool state);
        static void SetMouseState(MouseButton key, bool state);

        static void SetMousePosition(BlVec2<f32> position);
        static void SetScrollLevel(f32 level);
        static void ResetKeyState();

    };

} // namespace Blackberry