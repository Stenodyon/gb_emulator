#include "stdafx.h"
#include "Timer.h"
#include "CPU.h"

static double period_table[4] = {
	244.140625,
	3.81469727,
	15.2587891,
	61.0351563
};

void Timer::OnMachineCycle(uint64_t cycles)
{
	double elapsedTime = cycles * 4 * 0.238418579;
	dividerElapsed += elapsedTime;
	if (start)
		counterElapsed += elapsedTime;
}

void Timer::updateDivider()
{
	double remainder = dividerElapsed - 61.0351563;
	if (remainder >= 0)
	{
		divider++;
		dividerElapsed = remainder;
	}
}

void Timer::updateCounter()
{
	double period = period_table[clock_select];
	double remainder = counterElapsed - period;
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
	updateDivider();
	if (start)
		updateCounter();
}
