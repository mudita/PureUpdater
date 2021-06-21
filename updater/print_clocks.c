#include <stdio.h>
#include <drivers/fsl_clock.h>

typedef enum
{
        PerphClock_I2C,
        PerphClock_LPSPI,
        PerphClock_LPUART,
        PerphClock_SAI1,
        PerphClock_SAI2,
        PerphClock_USDHC2,
} PerphClock_t;

static uint32_t GetPerphSourceClock(PerphClock_t clock)
{
    switch (clock) {

    case PerphClock_I2C:
        return CLOCK_GetFreq(kCLOCK_OscClk) / (CLOCK_GetDiv(kCLOCK_Lpi2cDiv) + 1);
    case PerphClock_LPSPI:
        return CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk) / (CLOCK_GetDiv(kCLOCK_LpspiDiv) + 1);
    case PerphClock_LPUART:
        return CLOCK_GetFreq(kCLOCK_OscClk) / (CLOCK_GetDiv(kCLOCK_UartDiv) + 1);
    case PerphClock_SAI1:
        return CLOCK_GetFreq(kCLOCK_AudioPllClk) / (CLOCK_GetDiv(kCLOCK_Sai1Div) + 1) /
               (CLOCK_GetDiv(kCLOCK_Sai1PreDiv) + 1);
    case PerphClock_SAI2:
        return CLOCK_GetFreq(kCLOCK_AudioPllClk) / (CLOCK_GetDiv(kCLOCK_Sai2Div) + 1) /
               (CLOCK_GetDiv(kCLOCK_Sai2PreDiv) + 1);
    case PerphClock_USDHC2:
        return CLOCK_GetFreq(kCLOCK_SysPllPfd2Clk) / (CLOCK_GetDiv(kCLOCK_Usdhc2Div) + 1U);
    default:
        return 0;
    }
}


void PrintSystemClocks()
{
    const char *_PLLNames[22] = {
        "kCLOCK_CpuClk",  /*!< CPU clock */
        "kCLOCK_AhbClk",  /*!< AHB clock */
        "kCLOCK_SemcClk", /*!< SEMC clock */
        "kCLOCK_IpgClk",  /*!< IPG clock */
        "kCLOCK_OscClk", /*!< OSC clock selected by PMU_LOWPWR_CTRL[OSC_SEL]. */
        "kCLOCK_RtcClk", /*!< RTC clock. (RTCCLK) */
        "kCLOCK_ArmPllClk", /*!< ARMPLLCLK. */
        "kCLOCK_Usb1PllClk",     /*!< USB1PLLCLK. */
        "kCLOCK_Usb1PllPfd0Clk", /*!< USB1PLLPDF0CLK. */
        "kCLOCK_Usb1PllPfd1Clk", /*!< USB1PLLPFD1CLK. */
        "kCLOCK_Usb1PllPfd2Clk", /*!< USB1PLLPFD2CLK. */
        "kCLOCK_Usb1PllPfd3Clk", /*!< USB1PLLPFD3CLK. */
        "kCLOCK_Usb2PllClk", /*!< USB2PLLCLK. */
        "kCLOCK_SysPllClk",     /*!< SYSPLLCLK. */
        "kCLOCK_SysPllPfd0Clk", /*!< SYSPLLPDF0CLK. */
        "kCLOCK_SysPllPfd1Clk", /*!< SYSPLLPFD1CLK. */
        "kCLOCK_SysPllPfd2Clk", /*!< SYSPLLPFD2CLK. */
        "kCLOCK_SysPllPfd3Clk", /*!< SYSPLLPFD3CLK. */
        "kCLOCK_EnetPll0Clk", /*!< Enet PLLCLK ref_enetpll0. */
        "kCLOCK_EnetPll1Clk", /*!< Enet PLLCLK ref_enetpll1. */
        "kCLOCK_AudioPllClk", /*!< Audio PLLCLK. */
        "kCLOCK_VideoPllClk", /*!< Video PLLCLK. */
    };
    int i;
    for (i = 0; i < 22; i++) {
        printf("%s: %lu Hz\r\n", _PLLNames[i], CLOCK_GetFreq((clock_name_t)i));
    }
    printf("PerphSourceClock_I2C: %lu\r\n", GetPerphSourceClock(PerphClock_I2C));
    printf("PerphSourceClock_LPSPI: %lu\r\n", GetPerphSourceClock(PerphClock_LPSPI));
    printf("PerphSourceClock_LPUART: %lu\r\n", GetPerphSourceClock(PerphClock_LPUART));
    printf("PerphSourceClock_SAI1: %lu\r\n", GetPerphSourceClock(PerphClock_SAI1));
    printf("PerphSourceClock_SAI2: %lu\r\n", GetPerphSourceClock(PerphClock_SAI2));
    printf("PerphSourceClock_USDHC2: %lu\r\n", GetPerphSourceClock(PerphClock_USDHC2));
}