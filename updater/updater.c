#include <console/console.h>
#include <fsl_rtwdog.h>
#include <boot/pin_mux.h>
#include <boot/clock_config.h>

int main()
{   
    BOARD_InitBootloaderPins();
    RTWDOG_Disable(RTWDOG);
    RTWDOG_Deinit(RTWDOG);
    BOARD_InitBootClocks();
    debug_console_init();
    for(;;) {
        debug_console_write("Ala ma kota\r\n", 13);
    }
    return 0;
}