#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

void Display::update()
{
	status.ly_coincidence = LY() == lyc;
	if (status.coincidence_int && status.ly_coincidence)
		ram.cpu->interrupt(0x48);
	static uint64_t lastCycle = ram.cpu->cycleCount;
	if ((uint64_t)((ram.cpu->cycleCount - lastCycle) * 0.23866) > 109)
	{
		if (lcd_display_enable)
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			if (bg_display)
				drawBG();
			else
				SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
		}
		else
		{
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
		}

		ram.cpu->interrupt(0x40);
		if (status.vblank_int)
			ram.cpu->interrupt(0x48);
		lastCycle = ram.cpu->cycleCount;
	}
}
