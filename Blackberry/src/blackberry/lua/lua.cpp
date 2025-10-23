#include "blackberry/lua/lua.hpp"
#include "blackberry/core/log.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

namespace Blackberry::Lua {

    static lua_State* s_LState = nullptr;

    void Initialize() {
        s_LState = luaL_newstate();
        luaL_openlibs(s_LState);
    }

    void RunFile(const std::filesystem::path& path) {
        std::string strPath = path.string();

        if (luaL_dofile(s_LState, strPath.c_str()) != LUA_OK) {
            const char* errorMsg = lua_tostring(s_LState, -1);

            BL_ERROR("Lua Error in file '{}': {}", strPath, errorMsg);

            // Handle the error (e.g., log it)
            lua_pop(s_LState, 1); // Remove error message from stack
        }
    }

    void Shutdown() {
        if (s_LState) {
            lua_close(s_LState);
            s_LState = nullptr;
        }
    }

} // namespace Blackberry::Lua