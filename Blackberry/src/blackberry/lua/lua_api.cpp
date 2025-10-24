#include "blackberry/lua/lua_api.hpp"
#include "blackberry/core/log.hpp"
#include "blackberry/scene/entity.hpp"
#include "blackberry/lua/lua.hpp"

namespace Blackberry::Lua {

#pragma region LogModule

    static int WLogInfo(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);

        BL_INFO("[Lua] {}", msg);
        return 0;
    }

    static int WLogDebug(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        
        BL_DEBUG("[Lua] {}", msg);
        return 0;
    }

    static luaL_Reg LogModule[] = {
        { "Info", WLogInfo },
        { "Debug", WLogDebug },
        { nullptr, nullptr }
    };

    static int LoadLogModule(lua_State* L) {
        luaL_newlib(L, LogModule);

        return 1;
    }

#pragma endregion

#pragma region EntityModule

    static int WEntityGetComponent(lua_State* L) {
        Entity* entity = static_cast<Entity*>(lua_touserdata(L, 1));
        const char* componentName = luaL_checkstring(L, 2);

        if (!strcmp(componentName, "Transform")) {
            if (entity->HasComponent<Blackberry::Components::Transform>()) {
                Blackberry::Components::Transform& transform = entity->GetComponent<Blackberry::Components::Transform>();
                
                lua_newtable(L);
                lua_pushstring(L, "Position");
                Lua::PushVec2(transform.Position);
                lua_settable(L, -3);
                
                lua_pushstring(L, "Dimensions");
                Lua::PushVec2(transform.Dimensions);
                lua_settable(L, -3);
                
                return 1; // return the table
            } else {
                lua_pushnil(L);
                return 1; // return nil if component doesn't exist
            }
        };

        return 0;
    }

    static int WEntitySetComponent(lua_State* L) {
        Entity* entity = static_cast<Entity*>(lua_touserdata(L, 1));
        const char* componentName = luaL_checkstring(L, 2);

        if (!strcmp(componentName, "Transform")) {
            if (entity->HasComponent<Blackberry::Components::Transform>()) {
                Blackberry::Components::Transform& transform = entity->GetComponent<Blackberry::Components::Transform>();
                
                lua_getfield(L, 3, "Position");
                if (lua_istable(L, -1)) {
                    lua_getfield(L, -1, "x");
                    transform.Position.x = static_cast<f32>(lua_tonumber(L, -1));
                    lua_pop(L, 1);
                    
                    lua_getfield(L, -1, "y");
                    transform.Position.y = static_cast<f32>(lua_tonumber(L, -1));
                    lua_pop(L, 1);
                }
                lua_pop(L, 1); // pop Position table
                
                lua_getfield(L, 3, "Dimensions");
                if (lua_istable(L, -1)) {
                    lua_getfield(L, -1, "x");
                    transform.Dimensions.x = static_cast<f32>(lua_tonumber(L, -1));
                    lua_pop(L, 1);
                    
                    lua_getfield(L, -1, "y");
                    transform.Dimensions.y = static_cast<f32>(lua_tonumber(L, -1));
                    lua_pop(L, 1);
                }
                lua_pop(L, 1); // pop Dimensions table
            }
        }

        return 0;
    }

    static luaL_Reg EntityModule[] = {
        { "GetComponent", WEntityGetComponent },
        { "SetComponent", WEntitySetComponent },
        { nullptr, nullptr }
    };

    static int LoadEntityModule(lua_State* L) {
        luaL_newlib(L, EntityModule);

        return 1;
    }

#pragma endregion

    static int LoadBlackberryModule(lua_State* L) {
        // create Blackberry table
        lua_newtable(L);

        // create Log module
        lua_newtable(L);
        luaL_setfuncs(L, LogModule, 0);
        lua_setfield(L, -2, "Log"); // Blackberry.Log

        // create Entity module
        lua_newtable(L);
        luaL_setfuncs(L, EntityModule, 0);
        lua_setfield(L, -2, "Entity"); // Blackberry.Entity

        return 1;
    }

    void SetupApi(lua_State* L) {
        luaL_requiref(L, "Blackberry", LoadBlackberryModule, 1);
        lua_pop(L, 1); // remove module from stack
    }

} // namespace Blackberry::Lua