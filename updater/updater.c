#include "common/trace.h"
#include <hal/system.h>
#include <hal/delay.h>
#include <stdio.h>
#include <hal/display.h>
#include <hal/keyboard.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <procedure/package_update/update.h>
#include <procedure/backup/backup.h>
#include <common/enum_s.h>
#include <string.h>

static void main_status(trace_list_t *tl)
{
    trace_print(tl);
}

enum
{
    ErrMainOk,
    ErrMainVfs,
    ErrMainUpdate,
    ErrMainBackup,
};

const char* strerror_main(int val)
{
    switch (val) {
        ENUMS(ErrMainOk);
        ENUMS(ErrMainVfs);
        ENUMS(ErrMainUpdate);
        ENUMS(ErrMainBackup);
    }
    return "";
}

const char* strerror_main_ext(int val, int ext)
{
    switch (val) {
        case ErrMainOk:
            return "";
        case ErrMainVfs:
            return strerror(-ext);
    }
    return "";
}

int __attribute__((noinline, used)) main()
{
    trace_list_t tl;
    system_initialize();

    eink_clear_log();
    eink_log("Updater Init", false);

    tl = trace_init();
    trace_t *t = trace_append("main", &tl, strerror_main, strerror_main_ext);

    static const vfs_mount_point_desc_t fstab[] = {
        {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_fat, "/os"},
        {.disk = blkdev_emmc_user, .partition = 2, .type = vfs_fs_fat, "/backup"},
        {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_littlefs, "/user"},
    };

    int err = vfs_mount_init(fstab, sizeof fstab);
    if (err)
    {
        trace_write(t, ErrMainVfs, err);
        goto exit;
    }

    eink_log_printf("processing backup please wait!");

    struct backup_handle_s backup_handle;
    backup_handle.backup_from_os = "/os/current";
    backup_handle.backup_from_user = "/user";
    backup_handle.backup_to = "/backup/backup.tar";
    if(!backup_previous_firmware(&backup_handle, &tl))
    {
        trace_write(t, ErrMainBackup, 0);
        goto exit;
    }

    eink_log_printf("processing update please wait!");

    struct update_handle_s handle = {0,0,0,0};
    handle.update_from = "/os/update.tar";
    handle.update_os = "/os/current";
    handle.update_user = "/user";
    if (!update_firmware(&handle, &tl)) {
        trace_write(t, ErrMainUpdate, 0);
        goto exit;
    }

exit:
    main_status(&tl);
    eink_log_printf("update procedure status: %d", trace_list_ok(&tl));
    msleep(5000);
    printf("Before device free\n");
    err = vfs_unmount_deinit();
    printf("VFS subsystem free status %i\n", err);

    /*** Positive return code from main function 
     * or call exit with positive argument
     * casues a system reboot. Zero or negative value
     * only halts the system permanently
     */
    return err;
}
