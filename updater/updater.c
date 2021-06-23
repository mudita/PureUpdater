#include <hal/system.h>
#include <hal/delay.h>
#include <stdio.h>
#include <drivers/sdmmc/fsl_mmc.h>
#include <hal/display.h>
#include <hal/keyboard.h>
#include <hal/blk_dev.h>


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
    blk_dev_info_t info = {};
    error = blk_info( blk_disk_handle(blkdev_emmc_boot1,0), &info );
    printf("BOOT1 Sector count %lu sector size %lu error %i\n",info.sector_count, info.sector_size, error);
    error = blk_info( blk_disk_handle(blkdev_emmc_user,0), &info );
    printf("USER Sector count %lu sector size %lu error %i\n",info.sector_count, info.sector_size, error);

    
    for(;;) {
        kbd_event_t kevt;
        int err = kbd_read_key( &kevt );
        printf("jiffiess %lu kbdcode %i evttype %i err %i\n",
            get_jiffiess(), kevt.key, kevt.event, err );
        msleep(1000);
    }
    return 0;
}
