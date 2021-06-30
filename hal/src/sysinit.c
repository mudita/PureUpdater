#include <hal/system.h>
#include <fsl_rtwdog.h>
#include <boot/pin_mux.h>
#include <boot/clock_config.h>
#include <hal/delay.h>
#include <hal/console.h>
#include <hal/emmc.h>
#include <hal/display.h>
#include <hal/i2c_host.h>
#include <hal/i2c_dev_priv.h>
#include <hal/keyboard.h>
#include <boot/board.h>
#include <stdio.h>

void PrintSystemClocks();

static struct hal_i2c_dev i2c_gen = {.base = (uintptr_t)BOARD_KEYBOARD_I2C_BASEADDR, .initialized = false };


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
    //PrintSystemClocks();
    // Initialize emmc card
    if( emmc_init() ) {
        printf("Fatal: Unable to init EMMC card\n");
    }
    //Initialize Eink display
    eink_init();
    // Initialize the I2c controller
    if(!get_i2c_controller()) {
        printf( "Unable to intialize i2c device %i\n", i2c_gen.error);
    }
    if( kbd_init() ) {
        printf("Unable to initialize keyboard");
    }
}



/** Get I2C bus controller */
struct hal_i2c_dev* get_i2c_controller()
{
    if( !i2c_gen.initialized ) {
        i2c_gen.error = hal_i2c_init(&i2c_gen, BOARD_KEYBOARD_I2C_CLOCK_FREQ);
        if( i2c_gen.error != kStatus_Success)
            i2c_gen.initialized = true;
    }
    return (i2c_gen.error==kStatus_Success)?(&i2c_gen):(NULL);
}