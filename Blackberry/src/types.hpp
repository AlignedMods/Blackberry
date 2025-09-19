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

struct Vector2 {
    f32 x, y;
};

struct Color {
    f32 r, g, b, a;
};

struct Vertex {
    Vector2 pos;
    Color color;
};

namespace Blackberry {

    struct Image {
        const void* data;
        i32 width, height;
        i32 channels;
    };

} // namespace Blackberry

struct Texture {
    u32 id; // OpenGL texture ID (TODO: add Vulkan support?)
    u32 width, height;
};


struct Sound {

};
