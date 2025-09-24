#pragma once

#include <cstddef>
#include <cstdint>

// rust style types (no i still don't like rust, although type names is something they got right)

// integer types
using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;

// floating point types
using f32 = float;
using f64 = double;

struct BlVec2 {
    inline BlVec2() 
        : x(0.0f), y(0.0f) {}

    inline explicit BlVec2(f32 x, f32 y) 
        : x(x), y(y) {}

    inline explicit BlVec2(i32 x, i32 y)
        : BlVec2(static_cast<f32>(x), static_cast<f32>(y)) {}

    f32 x, y;
};

struct BlColor {
    inline BlColor()
        : r(0), g(0), b(0), a(0xff) {}

    inline explicit BlColor(u8 r, u8 g, u8 b, u8 a)
        : r(r), g(g), b(b), a(a) {}

    u8 r, g, b, a;
};

using BlTexture = void*; // the funny (we don't know what a texture could be since we support different backends)