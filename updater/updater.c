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

int main()
{ // System initialize
    system_initialize();
    // Try to initialize EINK
    eink_clear_log();
    eink_log("Dzien dobry to moj log", false);
    eink_log("A to kolejna linia", false);
    eink_log("Nowy test 2", false);
    eink_log_refresh();

    // VFS SUBSYSTEM TESTS
    static const vfs_mount_point_desc_t fstab[] = {
        {.disk = blkdev_emmc_user, .partition = 1, .type = vfs_fs_fat, "/os"},
        {.disk = blkdev_emmc_user, .partition = 3, .type = vfs_fs_littlefs, "/user"},
    };
    printf("Before device init\n");
    int err = vfs_mount_init(fstab, sizeof fstab);
    printf("VFS subsystem init status %i\n", err);
    if (err)
    {
        for (;;)
        {
        };
    }
    FILE *file = fopen("/os/.boot.json", "r");
    printf("Fopen handle %p\n", file);
    if (!file)
    {
        for (;;)
        {
        }
    }
    // Rozmiar pliku
    err = fseek(file, 0, SEEK_END);
    printf("Fseek result %i\n", err);
    long size = ftell(file);
    printf("ftell result %li\n", size);
    err = fseek(file, 0, SEEK_SET);
    printf("Fseek2 result %i\n", err);

    char line[512];
    while (fgets(line, sizeof line, file))
    {
        printf("Line [%s]\n", line);
    }
    err = fclose(file);
    printf("Fclose result %i\n", err);

    DIR *dir = opendir("/os");
    printf("Open dir status %p errno %i\n", dir, errno);
    if (!dir)
    {
        for (;;)
        {
        }
    }
    struct dirent *dent;
    while ((dent = readdir(dir)) != NULL)
    {
        printf("[%s]\n", dent->d_name);
    }
    err = closedir(dir);
    printf("closedir result %i\n", err);

    // Power off the VFS
    msleep(5000);
    printf("Before device free\n");
    err = vfs_unmount_deinit();
    printf("VFS subsystem free status %i\n", err);

    // Infinite loop test only
    for (;;)
    {
    }

    // Main loop read keys etc
    for (;;)
    {
        kbd_event_t kevt;
        int err = kbd_read_key(&kevt);
        printf("jiffiess %u kbdcode %i evttype %i err %i\n",
               (unsigned)get_jiffiess(), kevt.key, kevt.event, err);
        msleep(1000);
    }
    return 0;
}
