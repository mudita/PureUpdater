// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "lsyspaths.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM >= 502
#define new_lib(L, l) (luaL_newlib(L, l))
#else
#define new_lib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#endif

static int _get_path_os(lua_State *L) {
    lua_pushstring(L, "/os");
    return 1;
}

static int _get_path_user(lua_State *L) {
    lua_pushstring(L, "/user");
    return 1;
}

static int _get_path_backup(lua_State *L) {
    lua_pushstring(L, "/backup");
    return 1;
}

static const struct luaL_Reg lib[] = {
        {"os",     _get_path_os},
        {"user",   _get_path_user},
        {"backup", _get_path_backup},
        {NULL, NULL},
};

LUALIB_API int luaopen_lsyspaths(lua_State *L) {
    new_lib(L, lib);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "lsyspaths");
    return 1;
}