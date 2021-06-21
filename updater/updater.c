#include <hal/sysinit.h>
#include <hal/delay.h>
#include <hal/emmc.h>
#include <stdio.h>
#include <drivers/sdmmc/fsl_mmc.h>
#include <hal/display.h>

unsigned char buffer[512];

int main()
{   // System initialize
    sysinit_setup();
    // Main loop
    printf("Ta karta EMMC chyba dziala %p\n", emmc_card());

    // Try to retrive and read sector
    printf("Read status %li\n",MMC_ReadBlocks(emmc_card(),buffer,0,1));

    // Try to initialize EINK
    eink_init();
     eink_clear_log();
    eink_log( "Dzien dobry to moj log", true); 
    eink_log( "A to kolejna linia", true); 
    
    for(;;) {
        printf("test lepszego sprzetu %lu\n", get_jiffiess());
        msleep(1000);
    }
    return 0;
}
