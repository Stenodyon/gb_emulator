#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

void Display::update()
{
	SDL_RenderClear(renderer);
	static uint64_t lastCycle = ram.cpu->cycleCount;
	if ((uint64_t)((ram.cpu->cycleCount - lastCycle) * 0.23866) > 109)
	{
		drawBG();
		ram.cpu->interrupt(0x40);
		lastCycle = ram.cpu->cycleCount;
	}
	SDL_RenderPresent(renderer);
}
