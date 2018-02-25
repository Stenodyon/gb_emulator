#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

void Display::update()
{
	std::string title = lcd_display_enable ? "LCD ON" : "LCD OFF";
	title += " | ";
	title += bg_win_tile_data_select ? "LOW TILE DATA" : "HIGH TILE DATA";
	title += " | ";
	title += bg_tilemap_select ? "BG HIGH TILEMAP" : "BG LOW TILEMAP";
	title += " | ";
	title += win_display_enable ? "WINDOW ENABLED" : "WINDOW DISABLED";
	SDL_SetWindowTitle(win, title.c_str());
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

	SDL_Event e;
	while (SDL_PollEvent(&e));
}
