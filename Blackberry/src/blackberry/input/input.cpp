#include "blackberry/input/input.hpp"
#include "blackberry/core/log.hpp"

#include <unordered_map>

namespace Blackberry {

    struct _InputState {
        // bool in STL can be sketchy sometimes so we use u8
        std::unordered_map<u32, u8> CurrentKeyState;
        std::unordered_map<u32, u8> PreviousKeyState;

        std::unordered_map<u32, u8> CurrentMouseState;
        std::unordered_map<u32, u8> PreviousMouseState;

        BlVec2 CurrentMousePosition;
        BlVec2 PreviousMousePosition;
        f32 ScrollLevel = 0.0f;
    };

    static _InputState InputState;

    bool Input::IsKeyDown(KeyCode key) {
        return InputState.CurrentKeyState[static_cast<u32>(key)] == 1;
    }

    bool Input::IsKeyPressed(KeyCode key) {
        return InputState.CurrentKeyState[static_cast<u32>(key)] == 1 && InputState.PreviousKeyState[static_cast<u32>(key)] == 0;
    }

    bool Input::IsMouseDown(MouseButton key) {
        return InputState.CurrentMouseState[static_cast<u32>(key)] == 1;
    }

    bool Input::IsMousePressed(MouseButton key) {
        return InputState.CurrentMouseState[static_cast<u32>(key)] == 1 && InputState.PreviousMouseState[static_cast<u32>(key)] == 0;
    }

    bool Input::IsMouseReleased(MouseButton key) {
        return InputState.CurrentMouseState[static_cast<u32>(key)] == 0 && InputState.CurrentMouseState[static_cast<u32>(key)] == 1;
    }

    BlVec2 Input::GetMousePosition() {
        return InputState.CurrentMousePosition;
    }

    BlVec2 Input::GetMouseDelta() {
        return InputState.CurrentMousePosition - InputState.PreviousMousePosition;
    }

    f32 Input::GetScrollLevel() {
        return InputState.ScrollLevel;
    }

    void Input::SetKeyState(KeyCode key, bool state) {
        InputState.CurrentKeyState[static_cast<u32>(key)] = state;
    }

    void Input::SetMouseState(MouseButton key, bool state) {
        InputState.CurrentMouseState[static_cast<u32>(key)] = state;
    }

    void Input::ResetKeyState() {
        for (auto&[key, state] : InputState.CurrentKeyState) {
            InputState.PreviousKeyState[key] = state;
        }

        for (auto&[key, state] : InputState.CurrentMouseState) {
            InputState.PreviousMouseState[key] = state;
        }

        InputState.PreviousMousePosition = InputState.CurrentMousePosition;
        InputState.ScrollLevel = 0.0f;
    }

    void Input::SetMousePosition(BlVec2 position) {
        InputState.CurrentMousePosition = position;
    }

    void Input::SetScrollLevel(f32 level) {
        InputState.ScrollLevel = level;
    }

} // namespace Blackberry