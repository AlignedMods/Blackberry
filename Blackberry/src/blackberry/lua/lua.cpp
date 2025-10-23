#include "blackberry/lua/lua.hpp"
#include "blackberry/core/log.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#include <unordered_map>

namespace Blackberry::Lua {

    static std::unordered_map<std::string, u32> s_LoadedModules;

    static lua_State* s_LState = nullptr;

    void Initialize() {
        s_LState = luaL_newstate();
        luaL_openlibs(s_LState);
    }

    void Shutdown() {
        if (s_LState) {
            lua_close(s_LState);
            s_LState = nullptr;
        }
    }

    void RunFile(const std::filesystem::path& path, const std::string& moduleName) {
        std::string strPath = path.string();

        if (luaL_dofile(s_LState, strPath.c_str()) != LUA_OK) {
            const char* errorMsg = lua_tostring(s_LState, -1);

            BL_ERROR("Lua Error in file '{}': {}", strPath, errorMsg);

            lua_pop(s_LState, 1); // Remove error message from stack
        }

        s_LoadedModules[moduleName] = luaL_ref(s_LState, LUA_REGISTRYINDEX);
    }

    void SetExecutionContext(const std::string& moduleName) {
        lua_rawgeti(s_LState, LUA_REGISTRYINDEX, s_LoadedModules[moduleName]);
    }

    void PushRetValue() {
        lua_gettop(s_LState);
    }

    void PushMember(const std::string& table, const std::string& member) {
        lua_getglobal(s_LState, table.c_str());
        lua_getfield(s_LState, -1, member.c_str());
        lua_remove(s_LState, -2); // Remove the table from the stack, leaving only the member
    }

    void PushMember(const std::string& member) {
        lua_getfield(s_LState, -1, member.c_str());
    }

    void PushNumber(f64 value) {
        lua_pushnumber(s_LState, value);
    }

    void CallFunction(u32 argCount, u32 returnCount) {
        lua_call(s_LState, argCount, returnCount);
    }

} // namespace Blackberry::Lua