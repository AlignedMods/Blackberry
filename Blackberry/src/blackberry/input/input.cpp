#include "blackberry/input/input.hpp"
#include "blackberry/core/log.hpp"

#include <unordered_map>

namespace Blackberry {

    struct InputState {
        // bool in STL can be sketchy sometimes so we use u8
        std::unordered_map<u32, u8> CurrentKeyState;
        std::unordered_map<u32, u8> PreviousKeyState;

        f32 ScrollLevel = 0.0f;
    };

    static InputState IState;

    bool Input::IsKeyDown(KeyCode key) {
        return IState.CurrentKeyState[static_cast<u32>(key)] == 1;
    }

    bool Input::IsKeyPressed(KeyCode key) {
        return IState.CurrentKeyState[static_cast<u32>(key)] == 1 && IState.PreviousKeyState[static_cast<u32>(key)] == 0;
    }

    f32 Input::GetScrollLevel() {
        return IState.ScrollLevel;
    }

    void Input::SetKeyState(KeyCode key, bool state) {
        IState.CurrentKeyState[static_cast<u32>(key)] = state;
    }

    void Input::ResetKeyState() {
        for (auto&[key, state] : IState.CurrentKeyState) {
            IState.PreviousKeyState[key] = state;
        }

        IState.ScrollLevel = 0.0f;
    }

    void Input::SetScrollLevel(f32 level) {
        IState.ScrollLevel = level;
    }

} // namespace Blackberry