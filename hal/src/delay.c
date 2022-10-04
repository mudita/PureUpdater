#include <hal/delay.h>
#include <drivers/fsl_pit.h>
#include <drivers/fsl_clock.h>
#include <drivers/fsl_common.h>

static volatile uint32_t PITCounter = 0;

void delay_init(void)
{
	pit_config_t config;
	PIT_GetDefaultConfig(&config);
	PIT_Init(PIT, &config);
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, MSEC_TO_COUNT(1U, CLOCK_GetFreq(kCLOCK_OscClk)));
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, UINT32_MAX); // Before enabling IRQs clear pending IRQs status register
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	NVIC_EnableIRQ(PIT_IRQn);
	PIT_StartTimer(PIT, kPIT_Chnl_0);
}

void __attribute__((used)) PIT_DriverIRQHandler(void)
{
    PITCounter++;
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
    /* Wait until the flag has been cleared, so that the PIT interrupt won't trigger twice */
    while (PIT_GetStatusFlags(PIT, kPIT_Chnl_0) & kPIT_TimerFlag);
}

uint32_t get_jiffiess(void)
{
	return PITCounter;
}

void msleep(uint32_t delay)
{
	uint32_t _pv = get_jiffiess();

	if (delay > (UINT32_MAX - _pv))
	{
		delay -= UINT32_MAX - _pv;
		while (get_jiffiess() < UINT32_MAX)
		{
#ifndef DEBUG
			asm volatile("wfi\n");
#endif
		}
		while (get_jiffiess() < delay)
		{
#ifndef DEBUG
			asm volatile("wfi\n");
#endif
		}
	}
	else
	{
		while (get_jiffiess() < _pv + delay)
		{
#ifndef DEBUG
			asm volatile("wfi\n");
#endif
		}
	}
}
