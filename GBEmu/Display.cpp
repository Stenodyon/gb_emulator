#include "stdafx.h"
#include "Display.h"
#include "CPU.h"

#include <cmath>
#include <algorithm>

static std::chrono::time_point<std::chrono::system_clock> last_refresh;
static bool last_lcd_display_enable = false;
static uint8_t last_control;

void Display::OnMachineCycle(uint64_t cycles)
{
	lastElapsedTime = elapsedTime;
	elapsedTime += cycles * 4 * 0.238418579;
}

uint8_t Display::getBGColor(uint8_t x, uint8_t y)
{
	const uint8_t tileX = x / 8;
	const uint8_t tileY = y / 8;
	const uint16_t tileIndex = tileX + 32 * tileY;
	const uint16_t tileData = bg_tilemap_select ? 0x9C000 : 0x9800;
	const uint8_t tileCode = *(ram.memory + tileData + tileIndex);
	tile * tile = getTile(tileCode);
	const uint8_t pixelX = x & 0x07;
	const uint8_t pixelY = y & 0x07;
	const uint8_t pixelIndex = pixelX + 8 * pixelY;
	const uint8_t colorCode = (*tile)[pixelIndex];
	const uint8_t color = bg_palette[colorCode];
	return color;
}

uint8_t Display::getWindowColor(uint8_t x, uint8_t y)
{
	const uint8_t tileX = x / 8;
	const uint8_t tileY = y / 8;
	const uint16_t tileIndex = tileX + 32 * tileY;
	const uint16_t tileData = win_tilemap_select ? 0x9C00 : 0x09800;
	const uint8_t tileCode = *(ram.memory + tileData + tileIndex);
	tile * tile = getTile(tileCode);
	const uint8_t pixelX = x & 0x07;
	const uint8_t pixelY = y & 0x07;
	const uint8_t pixelIndex = pixelX + 8 * pixelY;
	const uint8_t colorCode = (*tile)[pixelIndex];
	const uint8_t color = bg_palette[colorCode];
	return color;
}

uint8_t Display::getBGColorUnderPixel(uint8_t x, uint8_t y)
{
	return getBGColor(x + scrollX, y + scrollY);
}

uint8_t Display::getWinColorUnderPixel(uint8_t x, uint8_t y)
{
	return getWindowColor(x - (winPosX - 7), y - winPosY);
}

void Display::drawTileAt(tile * tile, int16_t x, int16_t y, palette * palette, bool transparency)
{
	for (uint8_t _y = 0; _y < 8; _y++)
	{
		if (y + _y < 0 || y + _y >= 144)
			continue;
		for (uint8_t _x = 0; _x < 8; _x++)
		{
			if (x + _x < 0 || x + _x >= 160)
				continue;
			uint8_t index = _x + 8 * _y;
			uint8_t colorIndex = (*tile)[index];
			uint8_t color = (*palette)[colorIndex];
			drawPixel(x + _x, y + _y, color, transparency);
		}
	}
}

void Display::drawSprite(sprite * sprite)
{
	palette * palette = sprite->palette ? &obj_palette1 : &obj_palette0;
	if (obj_size)
	{
		tile * lower = getTile(sprite->chr_code &= ~0x01);
		tile * upper = lower + 1;
		drawTileAt(lower, sprite->x - 0x08, sprite->y - 0x10 + 0x08, palette, true);
		drawTileAt(upper, sprite->x - 0x08, sprite->y - 0x10, palette, true);
	}
	else
	{
		tile * tile = getTile(sprite->chr_code);
		drawTileAt(tile, sprite->x - 0x08, sprite->y - 0x10, palette, true);
	}
}

void Display::drawSprites()
{
	static const uint16_t oam_base = 0xFE00;
	for (uint8_t index = 0; index < 40; index++)
	{
		sprite * _sprite = (sprite*)(ram.memory + oam_base) + index;
		drawSprite(_sprite);
	}
}

void Display::drawBGTileAt(tile * tile, int16_t x, int16_t y)
{
	drawTileAt(tile, x, y, &bg_palette);
}

void Display::drawBG()
{
	const uint16_t tileData = bg_tilemap_select ? 0x9C000 : 0x9800;
	for (uint8_t _y = 0; _y < 19; _y++)
	{
		uint8_t yIndex = ((uint64_t)_y + scrollY / 8) % 32;
		for (uint8_t _x = 0; _x < 21; _x++)
		{
			uint8_t xIndex = ((uint64_t)_x + scrollX / 8) % 32;
			uint16_t index = xIndex + 32 * yIndex;
			uint8_t tileIndex = *(ram.memory + tileData + index);
			tile * tile = getTile(tileIndex);
			drawBGTileAt(tile, -(scrollX % 8) + _x * 8, -(scrollY % 8) + _y * 8);
		}
	}
}

void Display::drawWindow()
{
	const uint8_t windowX = this->winPosX - 7;
	const uint8_t windowY = this->winPosY;
	const uint8_t maxX = std::max<uint8_t>(0, 21 - this->winPosX / 8);
	const uint8_t maxY = std::max<uint8_t>(0, 19 - this->winPosY / 8);
	const uint16_t tileData = win_tilemap_select ? 0x9C00 : 0x09800;
	for (uint8_t _y = 0; _y < maxY; _y++)
	{
		for (uint8_t _x = 0; _x < maxX; _x++)
		{
			uint16_t index = _x + 32 * _y;
			uint8_t tileIndex = *(ram.memory + tileData + index);
			tile * tile = getTile(tileIndex);
			drawBGTileAt(tile, windowX + 8 * _x, windowY + 8 * _y);
		}
	}
}

void Display::drawLine(uint8_t line)
{
	for (uint8_t x = 0; x < 160; x++)
	{
		if (win_display_enable && line >= winPosY && (x >= (winPosX - 7)))
		{
			uint8_t color = getWinColorUnderPixel(x, line);
			drawPixel(x, line, color);
		}
		else if (bg_display)
		{
			uint8_t color = getBGColorUnderPixel(x, line);
			drawPixel(x, line, color);
		}
	}
}

void Display::setWindowTitle()
{
	if (last_control != control)
	{
		std::string title = lcd_display_enable ? "LCD ON" : "LCD OFF";
		title += " | ";
		title += obj_dispay_enable ? "OBJ ENABLED" : "OBJ DISABLED";
		title += " | ";
		title += bg_tilemap_select ? "BG HIGH TILEMAP" : "BG LOW TILEMAP";
		title += " | ";
		title += win_display_enable ? "WINDOW ENABLED" : "WINDOW DISABLED";
		SDL_SetWindowTitle(win, title.c_str());
	}
	last_control = control;
}

void Display::update()
{
	//setWindowTitle();
	status.ly_coincidence = LY() == lyc;
	if (status.coincidence_int && status.ly_coincidence)
		ram.cpu->interrupt(0x48);
	if (lastElapsedTime < VBLANK_us)
	{
		double line_progress = std::fmod(elapsedTime, LINE_us);
		double last_line_progress = std::fmod(lastElapsedTime, LINE_us);
		if (line_progress > OAM_us && last_line_progress < OAM_us) // Just ended OAM period
		{
			status.stat_mode = 3;
		}
		else if (line_progress > HBLANK_us && last_line_progress < HBLANK_us) // Just reached HBLANK
		{
			status.stat_mode = 0;

			if (lcd_display_enable)
				drawLine(LY());

			if (status.hblank_int)
				ram.cpu->interrupt(0x48);
		}
		else if (last_line_progress > line_progress) // Just reached next line
		{
			status.stat_mode = 2;
			if (status.oam_int)
				ram.cpu->interrupt(0x48);
		}
	}
	if (elapsedTime > VBLANK_us && lastElapsedTime < VBLANK_us) // Just reached VBLANK
	{
		status.stat_mode = 1;
		ram.cpu->interrupt(0x40);
		if (status.vblank_int)
			ram.cpu->interrupt(0x48);

		if (lcd_display_enable && obj_dispay_enable)
			drawSprites();
		SDL_RenderPresent(renderer);

		/*
		if (lcd_display_enable)
		{
			auto now = std::chrono::system_clock::now();
			uint64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refresh).count();
			if (elapsed > 16)
			{
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				if (bg_display)
					drawBG();
				if (win_display_enable)
					drawWindow();
				if (obj_dispay_enable)
					drawSprites();
				
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
		//*/
	}

	double remainder = elapsedTime - REFRESH_us;
	if (remainder > 0) // END OF VBLANK
	{
		elapsedTime = remainder;
	}
	last_lcd_display_enable = lcd_display_enable;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_KEYUP:
		case SDL_KEYDOWN:
		{
			bool set = e.type == SDL_KEYDOWN;
			switch (e.key.keysym.sym)
			{
			case SDLK_j: ram.cpu->joypad.joypad.button_a = set; break;
			case SDLK_k: ram.cpu->joypad.joypad.button_b = set; break;
			case SDLK_u: ram.cpu->joypad.joypad.button_start = set; break;
			case SDLK_i: ram.cpu->joypad.joypad.button_select = set; break;
			case SDLK_w: ram.cpu->joypad.joypad.up = set; break;
			case SDLK_a: ram.cpu->joypad.joypad.left = set; break;
			case SDLK_s: ram.cpu->joypad.joypad.down = set; break;
			case SDLK_d: ram.cpu->joypad.joypad.right = set; break;
			}
			break;
		}
		case SDL_QUIT:
			ram.cpu->running = false;
			break;
		}
	}
}
