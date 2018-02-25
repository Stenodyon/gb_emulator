#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

static std::chrono::time_point<std::chrono::system_clock> last_refresh;
static bool last_lcd_display_enable = false;
static uint8_t last_control;

void Display::update()
{
	if (last_control != control)
	{
		std::string title = lcd_display_enable ? "LCD ON" : "LCD OFF";
		title += " | ";
		title += bg_win_tile_data_select ? "LOW TILE DATA" : "HIGH TILE DATA";
		title += " | ";
		title += bg_tilemap_select ? "BG HIGH TILEMAP" : "BG LOW TILEMAP";
		title += " | ";
		title += win_display_enable ? "WINDOW ENABLED" : "WINDOW DISABLED";
		SDL_SetWindowTitle(win, title.c_str());
	}
	last_control = control;
	status.ly_coincidence = LY() == lyc;
	if (status.coincidence_int && status.ly_coincidence)
		ram.cpu->interrupt(0x48);
	static uint64_t lastCycle = ram.cpu->cycleCount;
	if ((uint64_t)((ram.cpu->cycleCount - lastCycle) * 0.23866) > 109)
	{
		if (lcd_display_enable)
		{
			auto now = std::chrono::system_clock::now();
			uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refresh).count();
			if (elapsed > 16)
			{
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				if (bg_display)
					drawBG();
				else
					SDL_RenderClear(renderer);
				SDL_RenderPresent(renderer);
				last_refresh = now;
			}
		}
		else if(last_lcd_display_enable)
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
	last_lcd_display_enable = lcd_display_enable;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			ram.cpu->running = false;
	}
}
