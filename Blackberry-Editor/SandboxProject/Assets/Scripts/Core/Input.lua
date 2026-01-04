if Input then return end

Input = {}

-- KeyCode enum taken from Blackberry/input/keycodes.hpp
KeyCode = {
    None = 0,

    -- non-printable keys
    Escape = 1,
    Tab = 2,
    CapsLock = 3,
    Shift = 4,
    Ctrl = 5,
    Win = 6,
    Alt = 7,
    Space = 8,
    Left = 9,
    Down = 10,
    Up = 11,
    Right = 12,
    PageDown = 13,
    PageUp = 14,
    Del = 15,
    Home = 16,
    Backspace = 17,
    Enter = 18,

    -- printable-keys
    Num0 = 19,
    Num1 = 20,
    Num2 = 21,
    Num3 = 22,
    Num4 = 23,
    Num5 = 24,
    Num6 = 25,
    Num7 = 26,
    Num8 = 27,
    Num9 = 28,
    A = 29,
    B = 30,
    C = 31,
    D = 32,
    E = 33,
    F = 34,
    G = 35,
    H = 36,
    I = 37,
    J = 38,
    K = 39,
    L = 40,
    M = 41,
    N = 42,
    O = 43,
    P = 44,
    Q = 45,
    R = 46,
    S = 47,
    T = 48,
    U = 49,
    V = 50,
    W = 51,
    X = 52,
    Y = 53,
    Z = 54,

    -- function keys
    F1 = 55,
    F2 = 56,
    F3 = 57,
    F4 = 58,
    F5 = 59,
    F6 = 60,
    F7 = 61,
    F8 = 62,
    F9 = 63,
    F10 = 64,
    F11 = 65,
    F12 = 66
}

function Input.IsKeyPressed(keycode)
    return InternalCalls.Input.IsKeyPressed(keycode)
end

function Input.IsKeyDown(keycode)
    return InternalCalls.Input.IsKeyDown(keycode)
end

return Input
