#include "blackberry/lua/lua_api.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/lua/lua.hpp"

namespace Blackberry::Lua {

#pragma region LogModule

    static int WLogTrace(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);

        BL_TRACE("[Lua] {}", msg);
        return 0;
    }

    static int WLogInfo(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        
        BL_INFO("[Lua] {}", msg);
        return 0;
    }

    static int WLogWarn(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        
        BL_WARN("[Lua] {}", msg);
        return 0;
    }

    static int WLogError(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        
        BL_ERROR("[Lua] {}", msg);
        return 0;
    }

    static int WLogCritical(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        
        BL_CRITICAL("[Lua] {}", msg);
        return 0;
    }

    static luaL_Reg LogModule[] = {
        { "Trace", WLogTrace },
        { "Info", WLogInfo },
        { "Warn", WLogWarn },
        { "Error", WLogError },
        { "Critical", WLogCritical },
        { nullptr, nullptr }
    };

    static int LoadLogModule(lua_State* L) {
        luaL_newlib(L, LogModule);

        return 1;
    }

#pragma endregion

#pragma region TransformComponent

    static int WEntityGetTransformPosition(lua_State* L) {
        u64 handle = lua_tointeger(L, 1);
        Scene* scene = reinterpret_cast<Scene*>(lua_touserdata(L, 2));

        Entity e(scene->GetEntityFromUUID(handle), scene);
        BL_ASSERT(e.HasComponent<TransformComponent>(), "Entity does not contain transform!");

        Lua::PushVec3(e.GetComponent<TransformComponent>().Position);

        return 1;
    }

    static int WEntityGetTransformRotation(lua_State* L) {
        u64 handle = lua_tointeger(L, 1);
        Scene* scene = reinterpret_cast<Scene*>(lua_touserdata(L, 2));

        Entity e(scene->GetEntityFromUUID(handle), scene);
        BL_ASSERT(e.HasComponent<TransformComponent>(), "Entity does not contain transform!");

        Lua::PushVec3(glm::degrees(glm::eulerAngles(e.GetComponent<TransformComponent>().Rotation)));

        return 1;
    }

    static int WEntityGetTransformScale(lua_State* L) {
        u64 handle = lua_tointeger(L, 1);
        Scene* scene = reinterpret_cast<Scene*>(lua_touserdata(L, 2));

        Entity e(scene->GetEntityFromUUID(handle), scene);
        BL_ASSERT(e.HasComponent<TransformComponent>(), "Entity does not contain transform!");

        Lua::PushVec3(e.GetComponent<TransformComponent>().Scale);

        return 1;
    }

    static int WEntitySetTransformPosition(lua_State* L) {
        u64 handle = lua_tointeger(L, 1);
        Scene* scene = reinterpret_cast<Scene*>(lua_touserdata(L, 2));

        Entity e(scene->GetEntityFromUUID(handle), scene);
        BL_ASSERT(e.HasComponent<TransformComponent>(), "Entity does not contain transform!");

        auto& transform = e.GetComponent<TransformComponent>();

        if (lua_istable(L, 3)) {
            lua_getfield(L, 3, "x");
            transform.Position.x = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            
            lua_getfield(L, 3, "y");
            transform.Position.y = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);

            lua_getfield(L, 3, "z");
            transform.Position.z = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);
        }

        return 0;
    }

    static int WEntitySetTransformRotation(lua_State* L) {
        u64 handle = lua_tointeger(L, 1);
        Scene* scene = reinterpret_cast<Scene*>(lua_touserdata(L, 2));

        Entity e(scene->GetEntityFromUUID(handle), scene);
        BL_ASSERT(e.HasComponent<TransformComponent>(), "Entity does not contain transform!");

        auto& transform = e.GetComponent<TransformComponent>();

        BlVec3 eulerRot(0.0f);

        if (lua_istable(L, 3)) {
            lua_getfield(L, 3, "x");
            eulerRot.x = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            
            lua_getfield(L, 3, "y");
            eulerRot.y = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);

            lua_getfield(L, 3, "z");
            eulerRot.z = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);
        }

        transform.Rotation = BlQuat(glm::radians(eulerRot));

        return 0;
    }

    static int WEntitySetTransformScale(lua_State* L) {
        u64 handle = lua_tointeger(L, 1);
        Scene* scene = reinterpret_cast<Scene*>(lua_touserdata(L, 2));

        Entity e(scene->GetEntityFromUUID(handle), scene);
        BL_ASSERT(e.HasComponent<TransformComponent>(), "Entity does not contain transform!");

        auto& transform = e.GetComponent<TransformComponent>();

        if (lua_istable(L, 3)) {
            lua_getfield(L, 3, "x");
            transform.Scale.x = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);
            
            lua_getfield(L, 3, "y");
            transform.Scale.y = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);

            lua_getfield(L, 3, "z");
            transform.Scale.z = static_cast<f32>(lua_tonumber(L, -1));
            lua_pop(L, 1);
        }

        return 0;
    }

#pragma endregion

    static int WEntityAddComponent(lua_State* L) {
        Entity* entity = static_cast<Entity*>(lua_touserdata(L, 1));
        const char* componentName = lua_tostring(L, 2);

        return 0;
    }

    static luaL_Reg EntityModule[] = {
        { "GetTransformPosition", WEntityGetTransformPosition},
        { "GetTransformRotation", WEntityGetTransformRotation},
        { "GetTransformScale", WEntityGetTransformScale},

        { "SetTransformPosition", WEntitySetTransformPosition},
        { "SetTransformRotation", WEntitySetTransformRotation},
        { "SetTransformScale", WEntitySetTransformScale},

        { "AddComponent", WEntityAddComponent },
        { nullptr, nullptr }
    };

    static int LoadEntityModule(lua_State* L) {
        luaL_newlib(L, EntityModule);

        return 1;
    }

    static int LoadInternalCallsModule(lua_State* L) {
        // create InternalCalls table
        lua_newtable(L);

        // create Log module
        lua_newtable(L);
        luaL_setfuncs(L, LogModule, 0);
        lua_setfield(L, -2, "Log"); // InternalCalls.Log

        // create Entity module
        lua_newtable(L);
        luaL_setfuncs(L, EntityModule, 0);
        lua_setfield(L, -2, "Entity"); // InternalCalls.Entity

        return 1;
    }

    void SetupApi(lua_State* L) {
        luaL_requiref(L, "InternalCalls", LoadInternalCallsModule, 1);
        lua_pop(L, 1); // remove module from stack
    }

} // namespace Blackberry::Lua