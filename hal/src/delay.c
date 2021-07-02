#include <hal/delay.h>
#include <drivers/fsl_pit.h>
#include <drivers/fsl_clock.h>

static volatile uint32_t PITCounter = 0;
#define USEC_TO_COUNT(us, clockFreqInHz) (uint64_t)((uint64_t)us * clockFreqInHz / 1000000U)

void delay_init(void)
{
	pit_config_t config;
	PIT_GetDefaultConfig(&config);
	PIT_Init(PIT, &config);
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0, USEC_TO_COUNT(1000, CLOCK_GetFreq(kCLOCK_IpgClk)));
	// Before enabling IRQs clear pending irqs status register
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, UINT32_MAX);
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);
	NVIC_EnableIRQ(PIT_IRQn);
	PIT_StartTimer(PIT, kPIT_Chnl_0);
}

void __attribute__((used)) PIT_DriverIRQHandler(void)
{
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	PITCounter++;
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
		}
		while (get_jiffiess() < delay)
		{
		}
	}
	else
	{
		while (get_jiffiess() < _pv + delay)
		{
		}
	}
}
