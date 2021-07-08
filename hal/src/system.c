#include <hal/system.h>
#include <fsl_rtwdog.h>
#include <fsl_snvs_hp.h>
#include <fsl_snvs_lp.h>
#include <boot/pin_mux.h>
#include <boot/clock_config.h>
#include <hal/delay.h>
#include <hal/console.h>
#include <hal/emmc.h>
#include <hal/display.h>
#include <hal/i2c_host.h>
#include <prv/hal/i2c_dev.h>
#include <hal/keyboard.h>
#include <boot/board.h>
#include <stdio.h>

static struct hal_i2c_dev i2c_gen = {.base = (uintptr_t)BOARD_KEYBOARD_I2C_BASEADDR, .initialized = false};

/** Initialize basic system setup */
void system_initialize(void)
{
    BOARD_InitBootloaderPins();
    RTWDOG_Disable(RTWDOG);
    RTWDOG_Deinit(RTWDOG);
    BOARD_InitBootClocks();
    debug_console_init();
    emmc_enable();
    SNVS_LP_Init(SNVS);
    SNVS_HP_Init(SNVS);
    delay_init();
    // Initialize emmc card
    if (emmc_init())
    {
        printf("Fatal: Unable to init EMMC card\n");
    }
    //Initialize Eink display
    eink_init();
    // Initialize the I2c controller
    if (!get_i2c_controller())
    {
        printf("Unable to intialize i2c device %i\n", i2c_gen.error);
    }
    if (kbd_init())
    {
        printf("Unable to initialize keyboard");
    }
}

/** Get I2C bus controller */
struct hal_i2c_dev *get_i2c_controller()
{
    if (!i2c_gen.initialized)
    {
        i2c_gen.error = hal_i2c_init(&i2c_gen, BOARD_KEYBOARD_I2C_CLOCK_FREQ);
        if (i2c_gen.error != kStatus_Success)
            i2c_gen.initialized = true;
    }
    return (i2c_gen.error == kStatus_Success) ? (&i2c_gen) : (NULL);
}

/** Get system boot reason code */
enum system_boot_reason_code system_boot_reason(void)
{
    static const uint32_t eco_update_code = 0xbadc0000;
    static const uint32_t eco_recovery_code = 0xbadc0001;
    static const uint32_t eco_factory_rst_code = 0xbadc0002;
    const uint32_t boot_code = SNVS->LPGPR[0];
    SNVS->LPGPR[0] = 0;
    switch (boot_code)
    {
    case eco_update_code:
        return system_boot_reason_update;
    case eco_recovery_code:
        return system_boot_reason_recovery;
    case eco_factory_rst_code:
        return system_boot_reason_factory;
    default:
        return system_boot_reason_unknown;
    }
}

// Get the system boot reason str
const char *system_boot_reason_str(enum system_boot_reason_code code)
{
    switch (code)
    {
    case system_boot_reason_update:
        return "system_boot_reason_update";
    case system_boot_reason_recovery:
        return "system_boot_reason_recovery";
    case system_boot_reason_factory:
        return "system_boot_reason_factory";
    case system_boot_reason_unknown:
        return "system_boot_reason_unknown";
    default:
        return "not in enum system_boot_reason_code";
    }
}