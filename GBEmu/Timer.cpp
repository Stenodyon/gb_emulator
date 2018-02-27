#include "stdafx.h"
#include "Timer.h"
#include "CPU.h"

static uint64_t cycles[4] = {
	1024,
	16,
	64,
	256
};

void Timer::OnMachineCycle(uint64_t cycles)
{
	double elapsedTime = cycles * 4 * 0.238418579;
	dividerElapsed += elapsedTime;
	if (start)
		cyclesCounter += cycles * 4;
	else
		cyclesCounter = 0;
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
	uint64_t period = cycles[clock_select];
	uint64_t remainder = cyclesCounter - period;
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
		cyclesCounter = remainder;
	}
}

void Timer::update()
{
	updateDivider();
	if (start)
		updateCounter();
}
