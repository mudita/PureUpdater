#include <stdio.h>
#include <hal/system.h>
#include <hal/delay.h>
#include <hal/display.h>
#include <hal/keyboard.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <procedure/package_update/update.h>
#include <procedure/security/pgmkeys.h>
#include <procedure/factory/factory.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "common/trace.h"
#include "main_trace.h"

int __attribute__((noinline, used)) main()
{
    trace_list_t tl;
    system_initialize();
    printf("System boot reason code: %s\n", system_boot_reason_str(system_boot_reason()));

    eink_clear_log();
    eink_log("run updater", true);

    tl = trace_init();
    trace_t *t = trace_append("main", &tl, strerror_main, strerror_main_ext);

    static const vfs_mount_point_desc_t fstab[] = {
        {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_fat, .mount_point = "/os"},
        {.disk = blkdev_emmc_user, .partition = 2, .type = vfs_fs_littlefs, .mount_point = "/backup"},
        {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_littlefs, .mount_point = "/user"},
    };

    int err = vfs_mount_init(fstab, sizeof fstab);
    if (err)
    {
        trace_write(t, ErrMainVfs, err);
        goto exit;
    }

    struct update_handle_s handle;
    memset(&handle, 0, sizeof handle);
    handle.update_os = "/os/current";
    handle.update_user = "/user";
    handle.tmp_os = "/os/tmp";
    handle.tmp_user = "/user/tmp";

    switch (system_boot_reason())
    {
    case system_boot_reason_update:
    {
        eink_log("system update ", true);
        handle.update_from = "/user/update.tar";
        handle.backup_full_path = "/backup/backup.tar";
        handle.enabled.backup = true;
        handle.enabled.check_checksum = true;
        handle.enabled.check_sign = true;
        handle.enabled.check_version = true;
        if (!update_firmware(&handle, &tl))
        {
            eink_log("update failure", true);
            trace_write(t, ErrMainUpdate, 0);
            goto exit;
        } else {
            eink_log("update success", true);
        }
        if (handle.unsigned_tar)
        {
            eink_log_printf("Warn: OS Update package");
            eink_log_printf("is not signed by Mudita");
            msleep(1000);
        }
    }
    break;
    case system_boot_reason_recovery:
    {
        eink_log("system recovery ", true);
        handle.update_from = "/backup/backup.tar";
        handle.enabled.backup = false;
        handle.enabled.check_checksum = true;
        handle.enabled.check_sign = false;
        handle.enabled.check_version = false;
        if (!update_firmware(&handle, &tl))
        {
            trace_write(t, ErrMainRecovery, 0);
            goto exit;
        }
    }
    break;
    case system_boot_reason_factory:
    {
        eink_log("factory reset", true);
        const struct factory_reset_handle frhandle = {
            .user_dir = handle.update_user};
        if (!factory_reset(&frhandle, &tl))
        {
            trace_write(t, ErrMainFactory, 0);
            goto exit;
        }
    }
    break;
    case system_boot_reason_pgm_keys:
    {
        eink_log("burn keys", true);
        const struct program_keys_handle pghandle = {
            .srk_file = "/os/current/SRK_fuses.bin",
            .chksum_srk_file = "/os/current/SRK_fuses.bin.md5"};
        if (program_keys(&pghandle, &tl))
        {
            trace_write(t, ErrMainUpdate, 0);
        }
        unlink(pghandle.srk_file);
        unlink(pghandle.chksum_srk_file);
        goto exit;
    }
    break;
    default:
        printf("not handled %d", system_boot_reason());
        break;
    }

exit:
    main_status(&tl);
    eink_log_printf("updater status %s", trace_list_ok(&tl) == true ? "OK" : "FAIL");
    eink_log_refresh();
    msleep(5000);
    err = vfs_unmount_deinit();
    printf("status %i : procedure: %i\n", err, trace_list_ok(&tl));
    system_deinitialize();

    /*** Positive return code from main function 
     * or call exit with positive argument
     * casues a system reboot. Zero or negative value
     * only halts the system permanently
     */
    return err;
}
