#pragma once

#include "blackberry/core/types.hpp"

namespace Blackberry {

    class Random {
    public:
        static void Init();

        static u8 GetU8(u8 min, u8 max);
        static i8 GetI8(i8 min, i8 max);

        static u16 GetU16(u16 min, u16 max);
        static i16 GetI16(i16 min, i16 max);

        static u32 GetU32(u32 min, u32 max);
        static i32 GetI32(i32 min, i32 max);

        static u64 GetU64(u64 min, u64 max);
        static i64 GetI64(i64 min, i64 max);

        static f32 GetF32(f32 min, f32 max);
        static f64 GetF64(f64 min, f64 max);
    };

} // namespace Blackberry