#include "blackberry/lua/lua.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/lua/lua_api.hpp"
#include "blackberry/core/path.hpp"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

#include <unordered_map>

namespace Blackberry::Lua {

    static std::unordered_map<std::string, u32> s_LoadedModules;

    static lua_State* s_LState = nullptr;

    void Init() {
        s_LState = luaL_newstate();
        luaL_openlibs(s_LState);

        SetupApi(s_LState);
    }

    void Shutdown() {
        if (s_LState) {
            lua_close(s_LState);
            s_LState = nullptr;
        }
    }

    void RunFile(const FS::Path& path, const std::string& moduleName) {
        std::string strPath = path.String();

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

    void NewTable() {
        lua_newtable(s_LState);
    }

    void SetTable(i32 index) {
        lua_settable(s_LState, index);
    }

    void GetGlobal(const std::string& name) {
        lua_getglobal(s_LState, name.c_str());
    }

    void GetMember(const std::string& table, const std::string& member) {
        lua_getglobal(s_LState, table.c_str());
        lua_getfield(s_LState, -1, member.c_str());
    }

    void GetMember(const std::string& member) {
        lua_getfield(s_LState, -1, member.c_str());
    }

    i32 GetTop() {
        return lua_gettop(s_LState);
    }

    void PushString(const std::string& value) {
        lua_pushstring(s_LState, value.c_str());
    }

    void PushNumber(f64 value) {
        lua_pushnumber(s_LState, value);
    }

    void PushLightUserData(void* data) {
        lua_pushlightuserdata(s_LState, data);
    }

    void PushValue(i32 value) {
        lua_pushvalue(s_LState, value);
    }

    void PushVec2(BlVec2<f32> vec) {
        lua_newtable(s_LState);
        lua_pushstring(s_LState, "x");
        lua_pushnumber(s_LState, vec.x);
        lua_settable(s_LState, -3);

        lua_pushstring(s_LState, "y");
        lua_pushnumber(s_LState, vec.y);
        lua_settable(s_LState, -3);
    }

    void PushVec3(BlVec3<f32> vec) {
        lua_newtable(s_LState);
        lua_pushstring(s_LState, "x");
        lua_pushnumber(s_LState, vec.x);
        lua_settable(s_LState, -3);

        lua_pushstring(s_LState, "y");
        lua_pushnumber(s_LState, vec.y);
        lua_settable(s_LState, -3);

        lua_pushstring(s_LState, "z");
        lua_pushnumber(s_LState, vec.z);
        lua_settable(s_LState, -3);
    }

    i32 ToInteger(i32 index) {
        return lua_tointeger(s_LState, index);
    }

    f64 ToNumber(i32 index) {
        return lua_tonumber(s_LState, index);
    }

    std::string ToTypename(i32 index) {
        return luaL_typename(s_LState, index);
    }

    void Pop(i32 count) {
        lua_pop(s_LState, count);
    }

    void Remove(i32 index) {
        lua_remove(s_LState, index);
    }

    void CallFunction(u32 argCount, u32 returnCount) {
        if (lua_pcall(s_LState, argCount, returnCount, 0) != LUA_OK) {
            const char* errorMsg = lua_tostring(s_LState, -1);
            BL_ERROR("Lua Error during function call: {}", errorMsg);
            lua_pop(s_LState, 1); // Remove error message from stack
        }
    }

    u32 GetScriptRef(const std::string& moduleName) {
        return s_LoadedModules.at(moduleName);
    }

    void* GetLuaState() {
        return s_LState;
    }

} // namespace Blackberry::Lua