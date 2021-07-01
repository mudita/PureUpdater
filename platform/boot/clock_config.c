/*
 * The Clear BSD License
 * Copyright 2017 NXP
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fsl_common.h"
#include "boot/clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* ARM PLL configuration for RUN mode */
const clock_arm_pll_config_t armPllConfig = {.loopDivider = 100U};

/* SYS PLL configuration for RUN mode */
const clock_sys_pll_config_t sysPllConfig = {.loopDivider = 1U};

/* USB1 PLL configuration for RUN mode */
const clock_usb_pll_config_t usb1PllConfig = {.loopDivider = 0U};

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockRUN();
}

const clock_arm_pll_config_t armPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 100, /* PLL loop divider, Fout = Fin * 50 */
        .src = 0,           /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
};
const clock_sys_pll_config_t sysPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 1, /* PLL loop divider, Fout = Fin * ( 20 + loopDivider*2 + numerator / denominator ) */
        .numerator = 0,   /* 30 bit numerator of fractional loop divider */
        .denominator = 1, /* 30 bit denominator of fractional loop divider */
        .src = 0,         /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
};
const clock_usb_pll_config_t usb1PllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 0, /* PLL loop divider, Fout = Fin * 20 */
        .src = 0,         /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
};

static void BOARD_BootClockGate(void)
{
    /* Disable all unused peripheral clock */
    //    CCM->CCGR0 = 0x00C0000FU;
    //    CCM->CCGR1 = 0x30000000U | (0x3<<12);
    //    CCM->CCGR2 = 0xFF3F303FU;
    //    CCM->CCGR3 = 0xF0000330U;
    //    CCM->CCGR4 = 0x0000FF3CU;
    //    CCM->CCGR5 = 0xF003330FU;
    //    CCM->CCGR6 = 0x00FC0F00U;

    CCM->CCGR0 = 0xFFFFFFFFU;
    CCM->CCGR1 = 0xFFFFFFFFU;
    CCM->CCGR2 = 0xFFFFFFFFU;
    CCM->CCGR3 = 0xFFFFFFFFU;
    CCM->CCGR4 = 0xFFFFFFFFU;
    CCM->CCGR5 = 0xFFFFFFFFU;
    CCM->CCGR6 = 0xFFFFFFFFU;
}

void BOARD_BootClockRUN(void)
{
    /* Boot ROM did initialize the XTAL, here we only sets external XTAL OSC freq */
    CLOCK_SetXtalFreq(24000000U);
    CLOCK_SetRtcXtalFreq(32768U);

    CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 0x1); /* Set PERIPH_CLK2 MUX to OSC */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0x1);     /* Set PERIPH_CLK MUX to PERIPH_CLK2 */

    /* Setting the VDD_SOC to 1.5V. It is necessary to config AHB to 600Mhz */
    DCDC->REG3 = (DCDC->REG3 & (~DCDC_REG3_TRG_MASK)) | DCDC_REG3_TRG(0x12);

    CLOCK_InitArmPll(&armPllConfig); /* Configure ARM PLL to 1200M */
#ifndef SKIP_SYSCLK_INIT
    CLOCK_InitSysPll(&sysPllConfig); /* Configure SYS PLL to 528M */
#endif
#ifndef XIP_EXTERNAL_FLASH
    CLOCK_InitUsb1Pll(&usb1PllConfig); /* Configure USB1 PLL to 480M */
#endif
    CLOCK_SetDiv(kCLOCK_ArmDiv, 0x1); /* Set ARM PODF to 0, divide by 2 */
    CLOCK_SetDiv(kCLOCK_AhbDiv, 0x0); /* Set AHB PODF to 0, divide by 1 */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3); /* Set IPG PODF to 3, divede by 4 */

    CLOCK_SetMux(kCLOCK_PrePeriphMux, 0x3); /* Set PRE_PERIPH_CLK to PLL1, 1200M */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0x0);    /* Set PERIPH_CLK MUX to PRE_PERIPH_CLK */
    //CLOCK_SetMux(kCLOCK_PerclkMux, 0x0);

    /* Disable unused clock */
    BOARD_BootClockGate();

    /* Power down all unused PLL */
    CLOCK_DeinitAudioPll();
    CLOCK_DeinitVideoPll();
    CLOCK_DeinitEnetPll();
    CLOCK_DeinitUsb2Pll();

    /* Configure UART divider to default */
    CLOCK_SetMux(kCLOCK_UartMux, 1); /* Set UART source to OSC */
    CLOCK_SetDiv(kCLOCK_UartDiv, 0); /* Set UART divider to 1 */

    /* Init System pfd2. */
    //CLOCK_InitSysPfd(kCLOCK_Pfd2, 29);

    /* Deinit system pfd0, pfd1, pfd3*/
    CLOCK_DeinitSysPfd(kCLOCK_Pfd0);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd1);
    CLOCK_DeinitSysPfd(kCLOCK_Pfd3);

    /* Set SEMC_PODF. */
    CLOCK_SetDiv(kCLOCK_SemcDiv, 1); //divide by 2
    /* Set Semc alt clock source. */
    CLOCK_SetMux(kCLOCK_SemcAltMux, 0); //PLL2 PFD2
    /* Set Semc clock source. */
    CLOCK_SetMux(kCLOCK_SemcMux, 1); //SEMC_ALT

#if 1
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 29); //
    /* Set SEMC_PODF. */
    CLOCK_SetDiv(kCLOCK_SemcDiv, 1); //divide by 2
    /* Set Semc alt clock source. */
    CLOCK_SetMux(kCLOCK_SemcAltMux, 0); //PLL2 PFD2
    /* Set Semc clock source. */
    CLOCK_SetMux(kCLOCK_SemcMux, 1); //SEMC_ALT

    /* Set AHB_PODF. */
    CLOCK_SetDiv(kCLOCK_AhbDiv, 3);
    /* Set IPG_PODF. */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 1);
#else
#ifndef SKIP_SYSCLK_INIT
    /* Set SEMC_PODF. */
    CLOCK_SetDiv(kCLOCK_SemcDiv, 3); //7
    /* Set Semc alt clock source. */
    CLOCK_SetMux(kCLOCK_SemcAltMux, 0); //0
    /* Set Semc clock source. */
    CLOCK_SetMux(kCLOCK_SemcMux, 0); //0
#endif
#endif
#if !(defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Init Usb1 PLL. */
    CLOCK_InitUsb1Pll(&usb1PllConfig_BOARD_BootClockRUN);
    /* Init Usb1 pfd0. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 33);
    /* Init Usb1 pfd1. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 16);
    /* Init Usb1 pfd2. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
    /* Init Usb1 pfd3. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);
    /* Disable Usb1 PLL output for USBPHY1. */
    CCM_ANALOG->PLL_USB1 &= ~CCM_ANALOG_PLL_USB1_EN_USB_CLKS_MASK;
#endif
    /* Set LPSPI_PODF. */
    CLOCK_SetDiv(kCLOCK_LpspiDiv, 7);
    /* Set Lpspi clock source. */
    CLOCK_SetMux(kCLOCK_LpspiMux, 3); //1

    /* Update core clock */
    SystemCoreClockUpdate();
}
