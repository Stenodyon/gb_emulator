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

void Display::setColor(uint8_t color)
{
	uint8_t _color = (3 - color) << 6;
	SDL_SetRenderDrawColor(renderer, _color, _color, _color, 255);
}

void Display::drawPixel(uint8_t x, uint8_t y)
{
	SDL_Rect pixel{ x * 4, y * 4, 4, 4 };
	SDL_RenderFillRect(renderer, &pixel);
}

void Display::drawPixel(uint8_t x, uint8_t y, uint8_t color, bool transparency)
{
	if (transparency && color == 0)
		return;
	setColor(color);
	drawPixel(x, y);
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

tile * Display::getTile(uint8_t index)
{
	if (bg_win_tile_data_select)
		return (tile*)(ram.memory + 0x8000) + index;
	else
		return (tile*)(ram.memory + 0x9000) + (int8_t)index;
}

uint8_t Display::getBGColorUnderPixel(uint8_t x, uint8_t y)
{
	return getBGColor(x + scrollX, y + scrollY);
}

uint8_t Display::getWinColorUnderPixel(uint8_t x, uint8_t y)
{
	return getWindowColor(x - (winPosX - 7), y - winPosY);
}

void Display::getVisibleSprites(uint8_t line, sprite * buffer[], uint8_t & count)
{
	count = 0;
	sprite * oam_base = (sprite*)(ram.memory + 0xFE00);
	for (uint8_t index = 0; index < 40; index++)
	{
		sprite * sprite = oam_base + index;
		if (line >= sprite->y
			&& ((!obj_size && line < sprite->y + 8)
				|| (obj_size && line < sprite->y + 16)))
		{
			buffer[count] = sprite;
			count++;
			if (count == 10)
				break;
		}
	}
}

tile * Display::getSpriteTile(uint8_t index)
{
	return (tile*)(ram.memory + 0x8000) + index;
}

uint8_t Display::getSpriteColor(sprite * sprite, uint8_t x, uint8_t y)
{
	int16_t spriteX = sprite->x; // -0x08;
	int16_t spriteY = sprite->y; // - 0x10;
	if (x < spriteX || y < spriteY || x >= spriteX + 8)
		return 0;
	if (!obj_size)
	{
		if (y >= spriteY + 8)
			return 0;
		tile * _tile = getSpriteTile(sprite->chr_code);
		uint8_t relX = x - spriteX;
		uint8_t relY = y - spriteY;
		if (sprite->hor_flip)
			relX = 7 - relX;
		if (sprite->vert_flip)
			relY = 7 - relY;
		palette * palette = sprite->palette ? &obj_palette1 : &obj_palette0;
		uint8_t colorCode = (*_tile)[relX + 8 * relY];
		return (*palette)[colorCode];
	}
	else
	{
		//TODO 8x16 sprite pixel selection
		if (y >= spriteY + 16)
			return 0;
		return 0;
	}
}

void Display::drawLine(uint8_t line)
{
	sprite * visible_sprites[10];
	uint8_t visible_count = 0;
	if (obj_dispay_enable)
	{
		getVisibleSprites(line, visible_sprites, visible_count);
	}
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
		if (obj_dispay_enable)
		{
			for (uint8_t sprite_index = 0; sprite_index < visible_count; sprite_index++)
			{
				sprite * sprite = visible_sprites[sprite_index];
				uint8_t color = getSpriteColor(sprite, x, line);
				if (color != 0)
				{
					drawPixel(x - 0x08, line - 0x10, color);
					break;
				}
			}
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
	if (lastElapsedTime < VBLANK_us && elapsedTime < VBLANK_us)
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

		SDL_RenderPresent(renderer);
	}
	if (status.stat_mode == 1 && status.vblank_int)
		ram.cpu->interrupt(0x48);

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
