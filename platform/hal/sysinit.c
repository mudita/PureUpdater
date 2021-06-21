#include <hal/sysinit.h>
#include <fsl_rtwdog.h>
#include <boot/pin_mux.h>
#include <boot/clock_config.h>
#include <hal/delay.h>
#include <hal/console.h>
#include <hal/emmc.h>
#include <stdio.h>

void PrintSystemClocks();

/** Initialize basic system setup */
void sysinit_setup(void)
{
    BOARD_InitBootloaderPins();
    RTWDOG_Disable(RTWDOG);
    RTWDOG_Deinit(RTWDOG);
    BOARD_InitBootClocks();
    debug_console_init();
    emmc_enable();
    delay_init();
    //TODO temporary
    PrintSystemClocks();
    if( emmc_init() ) {
        printf("Fatal: Unable to init EMMC card\n");
    }
}
