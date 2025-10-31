#pragma once

#include "blackberry/input/keycodes.hpp"

namespace Blackberry {

    class Input {
    public:
        static bool IsKeyDown(KeyCode key);
        static bool IsKeyPressed(KeyCode key);

        static f32 GetScrollLevel();

        static void SetKeyState(KeyCode key, bool state);
        static void ResetKeyState();

        static void SetScrollLevel(f32 level);
    };

} // namespace Blackberry