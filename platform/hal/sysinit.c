#include <hal/sysinit.h>
#include <fsl_rtwdog.h>
#include <boot/pin_mux.h>
#include <boot/clock_config.h>
#include <hal/delay.h>
#include <hal/console.h>


/** Initialize basic system setup */
void sysinit_setup(void)
{
    BOARD_InitBootloaderPins();
    RTWDOG_Disable(RTWDOG);
    RTWDOG_Deinit(RTWDOG);
    BOARD_InitBootClocks();
    delay_init();
    debug_console_init();
}
