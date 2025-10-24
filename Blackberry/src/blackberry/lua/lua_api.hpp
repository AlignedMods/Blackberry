#pragma once

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
}

namespace Blackberry::Lua {

    void SetupApi(lua_State* L);

} // namespace Blackberry::Lua