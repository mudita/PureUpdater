#include <hal/sysinit.h>
#include <hal/delay.h>
#include <hal/emmc.h>
#include <stdio.h>
#include <drivers/sdmmc/fsl_mmc.h>

unsigned char buffer[512];

int main()
{   // System initialize
    sysinit_setup();
    // Main loop
    printf("Ta karta EMMC chyba dziala %p\n", emmc_card());

    // Try to retrive and read sector
    printf("Read status %li\n",MMC_ReadBlocks(emmc_card(),buffer,0,1));
    
    for(;;) {
        printf("test lepszego sprzetu %lu\n", get_jiffiess());
        msleep(1000);
    }
    return 0;
}
