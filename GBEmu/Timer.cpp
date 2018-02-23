#include "stdafx.h"
#include "Timer.h"
#include "CPU.h"

static uint8_t period_table[] = {
	244,
	4,
	15,
	61
};

void Timer::update()
{
	static uint64_t dividerCounter = cpu->cycleCount;
	if ((uint64_t)((cpu->cycleCount - dividerCounter) * 0.23866) > 61)
	{
		divider++;
		dividerCounter = cpu->cycleCount;
	}

	if (stop)
		return;
	uint8_t period = period_table[clock_select];
	static uint64_t timerCounter = cpu->cycleCount;
	if ((uint64_t)((cpu->cycleCount - timerCounter) * 0.23866) > period)
	{
		if (counter == 0xFF)
		{
			counter = modulo;
			cpu->interrupt(0x50);
		}
		else
		{
			counter++;
		}
	}
}
