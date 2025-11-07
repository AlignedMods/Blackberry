#include "blackberry/core/random.hpp"
#include "blackberry/core/util.hpp"

#include <random>

namespace Blackberry {

    struct _RandomState {
        std::mt19937 RandomEngine;

        std::uniform_int_distribution<u64> IntDistribution;
        std::uniform_real_distribution<> FloatDistribution;
    };

    static _RandomState RandomState;

    void Random::Init() {
        BL_ASSERT(0, "not supported!");
    }

    u8 Random::GetU8(u8 min, u8 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    i8 Random::GetI8(i8 min, i8 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    u16 Random::GetU16(u16 min, u16 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    i16 Random::GetI16(i16 min, i16 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    u32 Random::GetU32(u32 min, u32 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    i32 Random::GetI32(i32 min, i32 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    u64 Random::GetU64(u64 min, u64 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    i64 Random::GetI64(i64 min, i64 max) {
        BL_ASSERT(0, "not supported!");
        return 0;
    }

    f32 Random::GetF32(f32 min, f32 max) {
        BL_ASSERT(0, "not supported!");
        return 0.0f;
    }

    f64 Random::GetF64(f64 min, f64 max) {
        BL_ASSERT(0, "not supported!");
        return 0.0;
    }

} // namespace Blackberry