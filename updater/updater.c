#include <hal/system.h>
#include <hal/delay.h>
#include <hal/emmc.h>
#include <stdio.h>
#include <drivers/sdmmc/fsl_mmc.h>
#include <hal/display.h>
#include <hal/keyboard.h>

unsigned char buffer[512];

int main()
{   // System initialize
    sysinit_setup();
    // Main loop
    printf("Ta karta EMMC chyba dziala %p\n", emmc_card());

    // Try to retrive and read sector
    printf("Read status %li\n",MMC_ReadBlocks(emmc_card(),buffer,0,1));

    // Try to initialize EINK
    eink_clear_log();
    eink_log( "Dzien dobry to moj log", false); 
    eink_log( "A to kolejna linia", false); 
    eink_log_refresh();
    
    for(;;) {
        kbd_event_t kevt;
        int err = kbd_read_key( &kevt );
        printf("jiffiess %lu kbdcode %i evttype %i err %i\n",
            get_jiffiess(), kevt.key, kevt.event, err );
        msleep(1000);
    }
    return 0;
}
