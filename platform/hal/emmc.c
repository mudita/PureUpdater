#include <hal/emmc.h>
#include <drivers/sdmmc/fsl_mmc.h>
#include <drivers/sdmmc/fsl_sdmmc_host.h>
#include <errno.h>
#include <stdbool.h>

static mmc_card_t mmc_card;
static bool init_ok;

static void config_emmc(void)
{
    /*configure system pll PFD2 fractional divider to 18*/
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 0x12U);
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 1U);
}
 
int emmc_init(void) 
{
    config_emmc();
    return 0;
    /* Configure base eMMC parameters*/
    memset(&mmc_card, 0, sizeof(mmc_card));

    mmc_card.busWidth = kMMC_DataBusWidth8bit;
    mmc_card.busTiming = /*kMMC_HighSpeed200Timing;*/ kMMC_HighSpeedTiming;
    mmc_card.enablePreDefinedBlockCount = true;
    mmc_card.host.base = MMC_HOST_BASEADDR;
    mmc_card.host.sourceClock_Hz = MMC_HOST_CLK_FREQ;
    /* card detect type */
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif
    if (MMC_Init(&mmc_card) != kStatus_Success)
    {
        return -1;
    }
    init_ok = true;
    return 0;
}


struct _mmc_card* emmc_card()
{
    return (init_ok)?(&mmc_card):(NULL);
}
