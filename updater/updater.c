#include <hal/system.h>
#include <hal/delay.h>
#include <stdio.h>
#include <drivers/sdmmc/fsl_mmc.h>
#include <hal/display.h>
#include <hal/keyboard.h>
#include <hal/blk_dev.h>
#include <ff.h>
#include <lfs.h>
#include <prv/tinyvfs/lfs_diskio.h>
#include <hal/tinyvfs.h>

static FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path); /* Open the directory */
    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno); /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0)
                break; /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR)
            { /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path); /* Enter the directory */
                if (res != FR_OK)
                    break;
                path[i] = 0;
            }
            else
            { /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }
    return res;
}

static int lfs_scan_files( lfs_t* lfs, char* path)
{
    lfs_dir_t lfs_dir;
    struct lfs_info lfs_info;
    int res = lfs_dir_open( lfs, &lfs_dir, path);
    if (res == LFS_ERR_OK)
    {
        for (;;)
        {
            res = lfs_dir_read(lfs, &lfs_dir, &lfs_info);
            if (res <= 0)
                break; /* Break on error or end of dir */
            if( lfs_info.type == LFS_TYPE_DIR && lfs_info.name[0]!='.' )
            {
                int i = strlen(path);
                sprintf(&path[i], "/%s", lfs_info.name);
                res = lfs_scan_files(lfs, path); /* Enter the directory */
                if (res <= 0)
                    break;
                path[i] = 0;
            }
            else
            { /* It is a file. */
                printf("%s/%s\n", path, lfs_info.name);
            }
        }
        lfs_dir_close(lfs, &lfs_dir);
    }
    return res;
}

int main()
{   // System initialize
    sysinit_setup();
    // Try to initialize EINK
    eink_clear_log();
    eink_log( "Dzien dobry to moj log", false); 
    eink_log( "A to kolejna linia", false); 
    eink_log_refresh();
    // Block device system initalize
    int error = blk_initialize();
    printf("Blk device subsystem init status %i\n",error);
    if(error) {
        return EXIT_FAILURE;
    }
    blk_dev_info_t info = {};
    error = blk_info( blk_disk_handle(blkdev_emmc_boot1,0), &info );
    printf("BOOT1 Sector count %lu sector size %lu error %i\n",info.sector_count, info.sector_size, error);
    error = blk_info( blk_disk_handle(blkdev_emmc_user,0), &info );
    printf("USER Sector count %lu sector size %lu error %i\n",info.sector_count, info.sector_size, error);
    // Test for get disc data
    blk_partition_t* parts;
    error = blk_get_partitions( blk_disk_handle(blkdev_emmc_user,0), &parts);
    if(error < 0) {
        printf("User get partitions error %i\n", error);
    }  else {
        printf("Number of partitions %i\n", error);
        for(int i=0; i<error; ++i ) {
            printf("Partition %i\n", i);
            printf("\tStart sector %lu type %u num_sectors %lu erase_siz %u\n", parts[i].start_sector, parts[i].type, parts[i].num_sectors, parts[i].erase_blk);
        }
    }
    if(1)
    {
        printf("Before mnt\n");
        FATFS *ffx = calloc(1, sizeof(FATFS));
        char scpath[LFS_NAME_MAX] = "1:";
        printf("ZZZZZZY MOUNT %i\n", f_mount(ffx,scpath , 1));
        int err = scan_files("1:");
        printf("Scan finished file error %i\n", err);
        f_unmount("1:");
        printf("To koniec odmontowuje ...\n");
        free(ffx);
    }
    if(1) {
        struct lfs_config cfg = {};
        lfs_t lfs = {};
        int err = vfs_lfs_append_volume(blk_disk_handle(blkdev_emmc_user,3),  &cfg);
        printf("LFS init status %i\n", err);
        if( err ) {
            return EXIT_FAILURE;
        }
        err = lfs_mount(&lfs, &cfg);
        char scpath[LFS_NAME_MAX+1] = "/";
        printf("LFS mount status %i\n", err);
        err = lfs_scan_files(&lfs,scpath);
        printf("Scan finished file error %i\n", err);
        err = lfs_unmount(&lfs);
        printf("LFS unmount status %i\n", err);
    }
   for(;;) {}
    for(;;) {
        kbd_event_t kevt;
        int err = kbd_read_key( &kevt );
        printf("jiffiess %lu kbdcode %i evttype %i err %i\n",
            get_jiffiess(), kevt.key, kevt.event, err );
        msleep(1000);
    }
    return 0;
}
