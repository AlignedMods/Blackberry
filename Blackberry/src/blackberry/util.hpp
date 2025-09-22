#pragma once

#include "blackberry/types.hpp"

#define MULTILINE_STR(...) #__VA_ARGS__

namespace Blackberry {

    inline BlColor HexToCol(const u32 hex) {
        BlColor color;

        color.r = static_cast<u8>(hex << 24) & 0xff;
        color.r = static_cast<u8>(hex << 16) & 0xff;
        color.r = static_cast<u8>(hex << 8)  & 0xff;
        color.r = static_cast<u8>(hex << 0)  & 0xff;

        return color;
    }

} // namespace Blackberry