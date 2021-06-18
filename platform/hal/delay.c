/*
 * delay.c
 *
 *  Created on: 30 lip 2018
 *      Author: crash
 */

#include <hal/delay.h>
#include <drivers/fsl_pit.h>

static volatile uint32_t PITCounter = 0;

void delay_init( void ){
	pit_config_t config;
	uint32_t busClock, busClockDivider;

	

	PIT_GetDefaultConfig(&config);
	PIT_Init(PIT, &config);

	busClock = CLOCK_GetFreq(kCLOCK_AhbClk);
	busClockDivider = CLOCK_GetDiv(kCLOCK_AhbDiv) + 1;	//According to NXP manual 000 -> 1 ... 111 -> 8
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, (busClock / 8 / busClockDivider) / 1000 - 1);

	// Before enabling IRQs clear pending irqs status register
	PIT_ClearStatusFlags(PIT,kPIT_Chnl_0,UINT32_MAX);
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	NVIC_EnableIRQ(PIT_IRQn);

	PIT_StartTimer(PIT, kPIT_Chnl_0);
}

void PIT_DriverIRQHandler(void) {
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);

	PITCounter++;
}

uint32_t get_jiffiess( void ) {
	return PITCounter;
}

void msleep(uint32_t delay) {
	uint32_t _pv = get_jiffiess();

	if (delay > (UINT32_MAX - _pv)) {
		delay -= UINT32_MAX - _pv;
		while (get_jiffiess() < UINT32_MAX);
		while (get_jiffiess() < delay);
	}
	else
		while (get_jiffiess() < _pv + delay);
}
