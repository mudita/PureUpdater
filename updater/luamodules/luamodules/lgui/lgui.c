// Copyright (c) 2017-2022, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include "lgui.h"
#include "lauxlib.h"
#include <gui/gui.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

#if LUA_VERSION_NUM >= 502
#define new_lib(L, l) (luaL_newlib(L, l))
#else
#define new_lib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#endif

static int _show_screen(lua_State *L) {
    const int screen_code = luaL_checkinteger(L, 1);
    gui_show_screen(screen_code);
    return 1;
}

static int _clear_display(lua_State *L) {
    UNUSED(L);
    gui_clear_display();
    return 1;
}

static const struct {
    const char *name;
    int value;
} gui_consts[] = {
        {"update_in_progress", ScreenUpdateInProgress},
        {"update_failed", ScreenUpdateFailed},
        {"update_success", ScreenUpdateSuccess},
        {"factory_reset_in_progress", ScreenFactoryResetInProgress},
        {"factory_reset_failed", ScreenFactoryResetFailed},
        {"factory_reset_success", ScreenFactoryResetSuccess},
        {"recovery_in_progress", ScreenRecoveryInProgress},
        {"recovery_failed", ScreenRecoveryFailed},
        {"recovery_success", ScreenRecoverySuccess},
        {"keys_in_progress", ScreenKeysInProgress},
        {"keys_failed", ScreenKeysFailed},
        {"keys_success", ScreenKeysSuccess},
        {"backup_in_progress", ScreenBackupInProgress},
        {"backup_failed", ScreenBackupFailed},
        {"backup_success", ScreenBackupSuccess},
        {"restore_in_progress", ScreenRestoreInProgress},
        {"restore_failed", ScreenRestoreFailed},
        {"restore_success", ScreenRestoreSuccess},
        {NULL, 0}
};

static const struct luaL_Reg guiLib[] = {
        {"show_screen", _show_screen},
        {"clear_display", _clear_display},
        {NULL, NULL}
};

LUALIB_API int luaopen_lgui(lua_State *L) {
    new_lib(L, guiLib);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "lgui");

    int i = 0;
    /* add constants to global table */
    while (gui_consts[i].name != NULL) {
        lua_pushstring(L, gui_consts[i].name);
        lua_pushinteger(L, gui_consts[i].value);
        lua_rawset(L, -3);
        ++i;
    }
    return 1;
}
