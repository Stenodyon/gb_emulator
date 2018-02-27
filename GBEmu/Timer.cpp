#include "stdafx.h"
#include "Timer.h"
#include "CPU.h"

// clock_select -> cycles
//	0 -> 1024
//	1 -> 16
//	2 -> 64
//	3 -> 256

void Timer::OnMachineCycle(uint64_t cycles)
{
	for (uint64_t cycle = 0; cycle < cycles * 4; cycle++)
	{
		bool prevInc = getTimerIncSignal();
		internal_counter++;
		bool inc = getTimerIncSignal();
		if (prevInc && !inc)
			incrementCounter();
	}
}

bool Timer::getTimerIncSignal()
{
	if (!start)
		return 0;
	uint8_t shift = 3 + 2 * ((clock_select + 3) % 4);
	return (internal_counter & (0x1 << shift));
}

void Timer::resetDivider()
{
	if (getTimerIncSignal())
	{
		std::cout << "div reset caused timer increment" << std::endl;
		incrementCounter();
	}
	internal_counter = 0;
}

void Timer::incrementCounter()
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

void Timer::setControlRegister(uint8_t value)
{
	bool prevInc = getTimerIncSignal();
	control = value;
	bool incSignal = getTimerIncSignal();
	if (prevInc && !incSignal)
		incrementCounter();
}
