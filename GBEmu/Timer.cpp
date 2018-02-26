#include "stdafx.h"
#include "Timer.h"
#include "CPU.h"

static double period_table[4] = {
	244.140625,
	3.81469727,
	15.2587891,
	61.0351563
};

void Timer::OnMachineCycle(uint8_t cycles)
{
	double elapsedTime = cycles * 4 * 0.238418579;
	dividerElapsed += elapsedTime;
	if (start)
		counterElapsed += elapsedTime;
}

void Timer::updateDivider()
{
	double remainder = 61.0351563 - dividerElapsed;
	if (remainder >= 0)
	{
		divider++;
		dividerElapsed = remainder;
	}
}

void Timer::updateCounter()
{
	double period = period_table[clock_select];
	double remainder = period - counterElapsed;
	if (remainder >= 0)
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
		counterElapsed = remainder;
	}
}

void Timer::update()
{
	static uint64_t dividerCounter = cpu->cycleCount;
	if ((cpu->cycleCount - dividerCounter) * 0.238418579 > 61.0351563)
	{
		divider++;
		dividerCounter = cpu->cycleCount;
	}

	if (!start)
		return;
	double period = period_table[clock_select];
	static uint64_t timerCounter = cpu->cycleCount;
	static uint64_t lastCycle = cpu->cycleCount;
	if ((cpu->cycleCount - timerCounter) * 0.238418579 > period)
	{
		if (counter == 0xFF)
		{
			std::cout << "Counter tick " << (lastCycle - cpu->cycleCount) << std::endl;
			counter = modulo;
			cpu->interrupt(0x50);
			lastCycle = cpu->cycleCount;
		}
		else
		{
			counter++;
		}
		timerCounter = cpu->cycleCount;
	}
}
