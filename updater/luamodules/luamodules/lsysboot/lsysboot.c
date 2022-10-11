// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "lsysboot.h"
#include "hal/boot_reason.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM >= 502
#define new_lib(L, l) (luaL_newlib(L, l))
#else
#define new_lib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#endif

#define SC(s)   { #s, system_boot_reason_ ## s },
static const struct {
    const char *name;
    int value;
} sysboot_consts[] = {
        /// Boot reason
        SC(update)
        SC(recovery)
        SC(factory)
        SC(pgm_keys)
        SC(unknown)
        /* terminator */
        {NULL, 0}
};

static int _get_reason(lua_State *L) {
    lua_pushinteger(L, system_boot_reason());
    return 1;
}

static int _get_reason_str(lua_State *L) {
    lua_pushstring(L, system_boot_reason_str(system_boot_reason()));
    return 1;
}

static int _get_reason_to_str(lua_State *L) {
    int code = luaL_checkinteger(L, 1);
    lua_pushstring(L, system_boot_reason_str(code));
    return 1;
}

static const struct luaL_Reg lib[] = {
        {"reason",        _get_reason},
        {"reason_str",    _get_reason_str},
        {"reason_to_str", _get_reason_to_str},
        {NULL, NULL},
};

LUALIB_API int luaopen_lsysboot(lua_State *L) {
    new_lib(L, lib);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "lsysboot");

    {
        int i = 0;
        /* add constants to global table */
        while (sysboot_consts[i].name) {
            lua_pushstring(L, sysboot_consts[i].name);
            lua_pushinteger(L, sysboot_consts[i].value);
            lua_rawset(L, -3);
            ++i;
        }
    }
    return 1;
}