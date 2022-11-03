#include <hal/system.h>
#include <hal/delay.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <hal/boot_control.h>
#include <string.h>

#include <database.h>
#include <log.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <lsqlite3/lsqlite3.h>
#include <luafilesystem/src/src/lfs.h>
#include <ltar/src/lmicrotar.h>
#include <lrecovery.h>

static const char *working_dir = "/user/tmp";
static const char *entry_point = "/user/tmp/main.lua";
static const char *boot_file = "/user/boot.json";
static const char *log_file = "/user/tmp/recovery.log";

static FILE *log_file_fd = NULL;

static lua_State *prepare_lua_context() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaL_requiref(L, "lsqlite3complete", luaopen_lsqlite3, 1);
    luaL_requiref(L, "lfs", luaopen_lfs, 1);
    luaL_requiref(L, "lmicrotar", luaopen_lmicrotar, 1);
    luaL_requiref(L, "lrecovery", luaopen_lrecovery, 1);
    return L;
}

static int invoke_entry_point(lua_State *L, const char *dir, const char *entry) {
    chdir(dir);
    return luaL_dofile(L, entry);
}

static int prepare_environment() {
    system_initialize();

    static const vfs_mount_point_desc_t fstab[] = {
            {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_auto, .mount_point = "/system_a"},
            {.disk = blkdev_emmc_user, .partition = 2, .type = vfs_fs_auto, .mount_point = "/system_b"},
            {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_auto, .mount_point = "/user"},
    };

    int err = vfs_mount_init(fstab, sizeof fstab);
    if (err) {
        debug_log("Unable to init vfs: %d", err);
        return err;
    }

    log_file_fd = freopen(log_file, "a+", stdout);

    database_initialize();

    err = boot_control_init(boot_file);
    if (err) {
        debug_log("Unable to init boot control module: %d", err);
        goto err;
    }

    return 0;

    err:
    database_deinitialize();
    err = vfs_unmount_deinit();
    system_deinitialize();
    return err;
}

int destroy_environment() {
    fclose(log_file_fd);
    database_deinitialize();
    int err = vfs_unmount_deinit();
    system_deinitialize();
    return err;
}

int __attribute__((noinline, used)) main() {
    int err = prepare_environment();
    if (err != 0) {
        return err;
    }

    lua_State *L = prepare_lua_context();
    err = invoke_entry_point(L, working_dir, entry_point);
    if (err) {
        debug_log("Error occurs when executing entry point, Hint Machine 0x%x", err);
        debug_log("Error: %s", lua_tostring(L, -1));
    }

    debug_log("Process finished, exiting...");
    destroy_environment();
    return err;
}
