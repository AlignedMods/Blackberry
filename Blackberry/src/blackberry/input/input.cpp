#include "blackberry/input/input.hpp"

#include <unordered_map>

namespace Blackberry {

    struct InputState {
        // bool in STL can be sketchy sometimes so we use u8
        std::unordered_map<u32, u8> CurrentKeyState;
        std::unordered_map<u32, u8> PreviousKeyState;

        f32 ScrollLevel = 0.0f;
    };

    static InputState State;

    bool Input::IsKeyDown(KeyCode key) {
        return State.CurrentKeyState[static_cast<u32>(key)] == 1;
    }

    bool Input::IsKeyPressed(KeyCode key) {
        return State.CurrentKeyState[static_cast<u32>(key)] == 1 && State.PreviousKeyState[static_cast<u32>(key)] == 0;
    }

    f32 Input::GetScrollLevel()
    {
        return f32();
    }

    void Input::SetKeyState(KeyCode key, bool state) {
        State.CurrentKeyState[static_cast<u32>(key)] = state;
    }

    void Input::ResetKeyState() {
        for (auto&[key, state] : State.CurrentKeyState) {
            State.PreviousKeyState[key] = state;
        }
    }

    void Input::SetScrollLevel(f32 level) {
        State.ScrollLevel = level;
    }

} // namespace Blackberry