#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

static std::chrono::time_point<std::chrono::system_clock> last_refresh;
static bool last_lcd_display_enable = false;
static uint8_t last_control;

void Display::drawBGTileAt(tile * tile, int32_t x, int32_t y)
{
	for (uint8_t _y = 0; _y < 8; _y++)
	{
		for (uint8_t _x = 0; _x < 8; _x++)
		{
			uint8_t index = _x + 8 * _y;
			uint8_t colorIndex = (*tile)[index];
			uint8_t color = getBackgroundColor(colorIndex);
			drawPixel(x + _x, y + _y, color);
		}
	}
}

void Display::drawBG()
{
	const uint16_t tileData = bg_tilemap_select ? 0x9C000 : 0x9800;
	for (uint8_t _y = 0; _y < 21; _y++)
	{
		uint8_t yIndex = ((uint64_t)_y + scrollY / 8) % 32;
		for (uint8_t _x = 0; _x < 19; _x++)
		{
			uint8_t xIndex = ((uint64_t)_x + scrollX / 8) % 32;
			uint16_t index = xIndex + 32 * yIndex;
			uint8_t tileIndex = *(ram.memory + tileData + index);
			//uint8_t tileIndex = _x + 32 * _y;
			tile * tile = getTile(tileIndex);
			drawBGTileAt(tile, -(scrollX % 8) + _x * 8, -(scrollY % 8) + _y * 8);
			//drawBGTileAt(tile, _x * 8, _y * 8);
		}
	}
}

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
