#pragma once

#include "blackberry/core/types.hpp"
#include "blackberry/core/log.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#define BL_STR(...) #__VA_ARGS__

#ifdef BL_DEBUG_BUILD
    #if defined(BL_WINDOWS_BUILD)
        #define BL_DEBUGBREAK() __debugbreak()
    #elif defined(BL_LINUX_BUILD)
        #include <signal.h>
        #define BL_DEBUGBREAK() raise(SIGTRAP)
    #endif
#else
    #define BL_DEBUGBREAK()
#endif

#if !0

// NOTE: blackberry asserts (BL_ASSERT) will always run the if statement AND exit program!
#if defined(BL_DEBUG_BUILD)
    #define BL_ASSERT(condition, message) if (!(condition)) { BL_ERROR("Assertion failed (Line: {}, File: {})!\nMessage: {}", __LINE__, __FILE__, message); BL_DEBUGBREAK(); }
#elif defined(BL_RELEASE_BUILD)
    #define BL_ASSERT(condition, message) if (!(condition)) { BL_ERROR("Assertion failed (Line: {}, File: {})!\nMessage: {}", __LINE__, __FILE__, message); exit(1); }
#endif

#else

#define BL_ASSERT(condition, message)

#endif

namespace Blackberry {

    inline BlColor HexToCol(const u32 hex) {
        BlColor color;

        color.r = static_cast<u8>(hex << 24) & 0xff;
        color.r = static_cast<u8>(hex << 16) & 0xff;
        color.r = static_cast<u8>(hex << 8)  & 0xff;
        color.r = static_cast<u8>(hex << 0)  & 0xff;

        return color;
    }

    inline std::string ReadEntireFile(const std::filesystem::path& path) {
        std::ifstream file(path);
        std::stringstream ss;
        ss << file.rdbuf();
        std::string contents = ss.str();
        ss.flush();

        return contents; // implicit move
    }

} // namespace Blackberry