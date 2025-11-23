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

#pragma region TemplateHell

template <typename T = f32>
struct BlVec2 {
    constexpr inline BlVec2() 
        : x{}, y{} {}

    template <typename B = T>
    constexpr inline explicit BlVec2(B scalar)
        : x(static_cast<T>(scalar)), y(static_cast<T>(scalar)) {}

    template <typename B = T>
    constexpr inline explicit BlVec2(B x, B y) 
        : x(static_cast<T>(x)), y(static_cast<T>(y)) {}

    constexpr inline BlVec2<T> operator+(BlVec2<T> rhs) {
        return BlVec2<T>(x + rhs.x, y + rhs.y);
    }

    constexpr inline BlVec2<T> operator-(BlVec2<T> rhs) {
        return BlVec2<T>(x - rhs.x, y - rhs.y);
    }

    constexpr inline BlVec2<T> operator*(BlVec2<T> rhs) {  
        return BlVec2<T>(x * rhs.x, y * rhs.y);
    }

    constexpr inline BlVec2<T> operator/(BlVec2<T> rhs) {  
        return BlVec2<T>(x / rhs.x, y / rhs.y);
    }

    constexpr inline void operator+=(BlVec2<T> rhs) {
        x += rhs.x;
        y += rhs.y;
    }

    constexpr inline void operator-=(BlVec2<T> rhs) {
        x -= rhs.x;
        y -= rhs.y;
    }

    constexpr inline void operator*=(BlVec2<T> rhs) {
        x *= rhs.x;
        y *= rhs.y;
    }

    constexpr inline void operator/=(BlVec2<T> rhs) {
        x /= rhs.x;
        y /= rhs.y;
    }

    constexpr inline bool operator==(BlVec2<T> rhs) {
        return x == rhs.x && y == rhs.y;
    }

    constexpr inline bool operator==(T scalar) {
        return x == scalar && y == scalar;
    }

    constexpr inline bool operator!=(BlVec2<T> rhs) {
        return x != rhs.x && y != rhs.y;
    }

    constexpr inline bool operator!=(T scalar) {
        return x != scalar && y != scalar;
    }

    T x, y;
};

template <typename T = f32>
struct BlVec3 {
    constexpr inline BlVec3()
        : x{}, y{}, z{} {}

    template <typename B = T>
    constexpr inline explicit BlVec3(B scalar)
        : x(static_cast<T>(scalar)), y(static_cast<T>(scalar)), z(static_cast<T>(scalar)) {}

    constexpr inline explicit BlVec3(BlVec2<T> vec)
        : x(vec.x), y(vec.y), z{} {}

    template <typename B = T>
    constexpr inline explicit BlVec3(B x, B y, B z)
        : x(static_cast<T>(x)), y(static_cast<T>(y)), z(static_cast<T>(z)) {}

    constexpr inline BlVec3<T> operator+(BlVec3<T> rhs) {
        return BlVec3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    constexpr inline BlVec3<T> operator-(BlVec3<T> rhs) {
        return BlVec3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    constexpr inline BlVec3<T> operator*(BlVec3<T> rhs) {
        return BlVec3<T>(x * rhs.x, y * rhs.y, z * rhs.z);
    }

    constexpr inline BlVec3<T> operator*(T scalar) {
        return BlVec3<T>(x * scalar, y * scalar, z * scalar);
    }

    constexpr inline BlVec3<T> operator/(BlVec3<T> rhs) {
        return BlVec3<T>(x / rhs.x, y / rhs.y, z / rhs.z);
    }

    constexpr inline void operator+=(BlVec3<T> rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
    }

    constexpr inline void operator-=(BlVec3<T> rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
    }

    constexpr inline void operator*=(BlVec3<T> rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
    }

    constexpr inline void operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
    }

    constexpr inline void operator/=(BlVec3<T> rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
    }

    constexpr inline bool operator==(BlVec3<T> rhs) {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }

    constexpr inline bool operator==(T scalar) {
        return x == scalar && y == scalar && z == scalar;
    }

    constexpr inline bool operator!=(BlVec3<T> rhs) {
        return x != rhs.x && y != rhs.y && z != rhs.z;
    }

    constexpr inline bool operator!=(T scalar) {
        return x != scalar && y != scalar && z != scalar;
    }

    T x, y, z;
};

template <typename T = f32>
struct BlVec4 {
    constexpr inline BlVec4()
        : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

    constexpr inline explicit BlVec4(T scalar)
        : x(scalar), y(scalar), z(scalar), w(scalar) {}

    constexpr inline explicit BlVec4(T x, T y, T z, T w)
        : x(x), y(y), z(z), w(w) {}

    T x, y, z, w;
};

#pragma endregion

struct BlColor {
    constexpr inline BlColor()
        : r(0), g(0), b(0), a(0xff) {}

    constexpr inline explicit BlColor(u8 r, u8 g, u8 b, u8 a)
        : r(r), g(g), b(b), a(a) {}

    u8 r, g, b, a;
};

struct BlRec {
    constexpr inline BlRec()
        : x(0.0f), y(0.0f), w(0.0f), h(0.0f) {}

    constexpr inline explicit BlRec(f32 x, f32 y, f32 w, f32 h)
        : x(x), y(y), w(w), h(h) {}

    constexpr inline explicit BlRec(i32 x, i32 y, i32 w, i32 h)
        : x(static_cast<f32>(x)), y(static_cast<f32>(y)), w(static_cast<f32>(w)), h(static_cast<f32>(h)) {}
    
    f32 x, y, w, h;
};