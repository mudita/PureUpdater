#include <hal/system.h>
#include <hal/boot_reason.h>
#include <hal/delay.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <procedure/package_update/update.h>
#include <procedure/security/pgmkeys.h>
#include <procedure/factory/factory.h>
#include <common/status_json.h>
#include <common/version_json.h>
#include <gui/gui.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

int __attribute__((noinline, used)) main() {
    system_initialize();

    gui_clear_display();

    static const vfs_mount_point_desc_t fstab[] = {
            {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_fat, .mount_point = "/os"},
            {.disk = blkdev_emmc_user, .partition = 2, .type = vfs_fs_auto, .mount_point = "/backup"},
            {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_auto, .mount_point = "/user"},
    };

    int err = vfs_mount_init(fstab, sizeof fstab);
    if (err) {
        printf("Unable to init vfs: %d", err);
        goto exit_no_save;
    }

    struct update_handle_s handle;
    memset(&handle, 0, sizeof handle);
    handle.update_os = "/os/current";
    handle.update_user = "/user";
    handle.tmp_os = "/os/tmp";
    handle.tmp_user = "/user/tmp";
    handle.current_version_json = "/os/current/version.json";
    handle.new_version_json = "/os/tmp/version.json";

    const struct version_json_s current_version_json = json_get_version_struct(handle.current_version_json);

    debug_log("****************************");
    debug_log("* MuditaOS updater v.%s *", current_version_json.updater.version);
    debug_log("****************************");
    debug_log("System boot reason code: %s", system_boot_reason_str(system_boot_reason()));

    struct status_json_s status;
    status.file_path = "/user/updater_status.json";
    status.updater_version = current_version_json.updater.version;
    status.performed_operation = status_json_boot_reason_to_operation_str(system_boot_reason());
    status.operation_result = OPERATION_SUCCESS;

    /* Remove previous status file in case process fails unexpectedly */
    if(!status_json_delete(&status)) {
        debug_log("Status JSON removing failed");
    }

    switch (system_boot_reason()) {
        case system_boot_reason_update: {
            debug_log("System update start");
            gui_show_screen(ScreenUpdateInProgress);

            handle.update_from = "/user/update.tar";
            handle.backup_full_path = "/backup/backup.tar";
            handle.enabled.backup = true;
            handle.enabled.check_checksum = true;
            handle.enabled.check_sign = true;
            handle.enabled.check_version = true;
            handle.enabled.allow_downgrade = false;

            if (!update_firmware(&handle)) {
                status.operation_result = OPERATION_FAILURE;
                debug_log("Update: update failed");
                gui_show_screen(ScreenUpdateFailed);
                goto exit;
            }

            debug_log("Update: update success");
            gui_show_screen(ScreenUpdateSuccess);
            if (handle.unsigned_tar) {
                debug_log("\n\nWarning: OS Update package is not signed by Mudita\n");
            }
        }
        break;

        case system_boot_reason_recovery: {
            debug_log("System recovery start");
            gui_show_screen(ScreenRecoveryInProgress);

            handle.update_from = "/backup/backup.tar";
            handle.enabled.backup = false;
            handle.enabled.check_checksum = true;
            handle.enabled.check_sign = false;
            handle.enabled.check_version = false;
            handle.enabled.allow_downgrade = false;

            if (!update_firmware(&handle)) {
                status.operation_result = OPERATION_FAILURE;
                debug_log("Recovery: recovery failed");
                gui_show_screen(ScreenRecoveryFailed);
                goto exit;
            }

            debug_log("Recovery: recovery success");
            gui_show_screen(ScreenRecoverySuccess);
        }
        break;

        case system_boot_reason_factory: {
            debug_log("Factory reset start");
            gui_show_screen(ScreenFactoryResetInProgress);

            const struct factory_reset_handle frhandle = {
                    .user_dir = handle.update_user
            };
            if (!factory_reset(&frhandle)) {
                status.operation_result = OPERATION_FAILURE;
                debug_log("Factory reset: factory reset failed");
                gui_show_screen(ScreenFactoryResetFailed);
                goto exit;
            }

            debug_log("Factory reset: factory reset success");
            gui_show_screen(ScreenFactoryResetSuccess);
        }
        break;

        case system_boot_reason_pgm_keys: {
            debug_log("Keys programming start");
            gui_show_screen(ScreenKeysInProgress);

            const struct program_keys_handle pghandle = {
                    .srk_file = "/os/current/SRK_fuses.bin",
                    .chksum_srk_file = "/os/current/SRK_fuses.bin.md5"
            };
            if (program_keys(&pghandle)) {
                status.operation_result = OPERATION_FAILURE;
                debug_log("Keys: burning keys failed");
                gui_show_screen(ScreenKeysFailed);
            }
            else {
                debug_log("Keys: burning keys success");
                gui_show_screen(ScreenKeysSuccess);
            }

            unlink(pghandle.srk_file);
            unlink(pghandle.chksum_srk_file);
        }
        break;

        default: {
            debug_log("Boot reason not handled: %d", system_boot_reason());
            goto exit_no_save;
        }
        break;
    }

    exit:
    debug_log("Saving status file");
    if(!status_json_save(&status)) {
        debug_log("Status file saving failed");
    }

    exit_no_save:
    debug_log("Process finished, exiting...");
    msleep(5000);
    gui_clear_display();
    err = vfs_unmount_deinit();
    system_deinitialize();

    /*** Positive return code from main function
     * or call exit with positive argument
     * casues a system reboot. Zero or negative value
     * only halts the system permanently
     */
    return err;
}
