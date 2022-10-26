// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "ldelay.h"
#include "lauxlib.h"
#include <hal/delay.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

#if LUA_VERSION_NUM >= 502
#define new_lib(L, l) (luaL_newlib(L, l))
#else
#define new_lib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#endif

static int _init(lua_State *L) {
    UNUSED(L);
    delay_init();
    return 1;
}

static int _msleep(lua_State *L) {
    const int ms = luaL_checkinteger(L, 1);
    msleep(ms);
    return 1;
}

static int _get_jiffiess(lua_State *L) {
    UNUSED(L);
    const int jiffiess = get_jiffiess();
    lua_pushinteger(L, jiffiess);
    return 1;
}

static const struct luaL_Reg delayLib[] = {
        {"init", _init},
        {"msleep", _msleep},
        {"get_jiffiess", _get_jiffiess},
        {NULL, NULL}
};

LUALIB_API int luaopen_ldelay(lua_State *L) {
    new_lib(L, delayLib);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "ldelay");
    return 1;
}
