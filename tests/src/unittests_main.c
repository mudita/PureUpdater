#include <hal/system.h>
#include <hal/delay.h>
#include <hal/display.h>
#include <hal/tinyvfs.h>
#include <hal/blk_dev.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <seatest/seatest.h>
#include "test_suite_vfs.h"

static void all_tests(void)
{
    test_fixture_vfs();
}

int main(void)
{
    // System initialize
    system_initialize();
    // Eink welcome message
    eink_clear_log();
    eink_log_printf("Unit test updater");
    eink_log_printf("Date %s %s", __DATE__, __TIME__);
    eink_log("Please wait...", false);
    eink_log_refresh();

    // fstab filesystem mounts
    static const vfs_mount_point_desc_t fstab[] = {
        {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_fat, "/os"},
        {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_littlefs, "/user"},
    };
    printf("Initializing VFS subsystem...\n");
    int err = vfs_mount_init(fstab, sizeof fstab);
    if (err)
    {
        printf("Failed to initialize VFS errno %i\n", err);
    }
    err = run_tests(all_tests);
    eink_log_printf("Finished with code %i", err);
    eink_log_refresh();
    err = vfs_unmount_deinit();
    if (err)
    {
        printf("Failed to umount VFS data errno %i\n", err);
    }
    return 0;
}