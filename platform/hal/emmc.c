#include <hal/emmc.h>
#include <drivers/sdmmc/fsl_mmc.h>
#include <drivers/sdmmc/fsl_sdmmc_host.h>
#include <errno.h>
#include <stdbool.h>

static mmc_card_t mmc_card;
static bool init_ok;

/** Enable the clocks in the emmc card 
 * @return error
 */
void emmc_enable(void)
{
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 2); //bylo 2
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 0); // CSCMR1  (17) 0 - PLL2_PFD2, 1 - PLL2_PFD0
    CLOCK_EnableClock(kCLOCK_Usdhc2);
}

/** Initialize the EMMC card */
int emmc_init(void) 
{
    //config_emmc();
    /* Configure base eMMC parameters*/
    memset(&mmc_card, 0, sizeof(mmc_card));

    mmc_card.busWidth = kMMC_DataBusWidth8bit;
    mmc_card.busTiming = /*kMMC_HighSpeed200Timing;*/ kMMC_HighSpeedTiming;
    mmc_card.enablePreDefinedBlockCount = true;
    mmc_card.host.base = MMC_HOST_BASEADDR;
    mmc_card.host.sourceClock_Hz =  MMC_HOST_CLK_FREQ;
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

/* Retrive the mmc card object structure
*/
struct _mmc_card* emmc_card()
{
    return (init_ok)?(&mmc_card):(NULL);
}
