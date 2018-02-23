#pragma once

#include <SDL.h>
#include <chrono>
#include <thread>

#include "RAM.h"

struct tile
{
	uint8_t data[16];

	uint8_t operator[](uint8_t pixelIndex)
	{
		uint8_t index = pixelIndex >> 3;
		uint8_t bitIndex = 0x7 - (pixelIndex & 0x7);
		uint8_t lower = data[index] >> (bitIndex - 1);
		uint8_t upper = data[index + 1] >> bitIndex;
		return upper | lower;
	}
};

static_assert(sizeof(tile) == 16, "tile structure is not 16 bytes");

class Display
{
private:
	SDL_Window * win;
	SDL_Renderer * renderer;
	RAM & ram;

	std::chrono::time_point<std::chrono::system_clock> startTime;
	uint64_t getMicroseconds()
	{
		auto diff = std::chrono::system_clock::now() - startTime;
		return std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
	}

	void setColor(uint8_t color)
	{
		uint8_t _color = (3 - color) << 6;
		SDL_SetRenderDrawColor(renderer, _color, _color, _color, 255);
	}

	void drawPixel(uint8_t x, uint8_t y)
	{
		SDL_Rect pixel{x * 4, y * 4, 4, 4};
		SDL_RenderDrawRect(renderer, &pixel);
	}

	void drawPixel(uint8_t x, uint8_t y, uint8_t color)
	{
		setColor(color);
		drawPixel(x, y);
	}

	tile * getTile(uint8_t index)
	{
		return (tile*)(ram.memory + 0x8000) + index;
	}

	uint8_t getBackgroundColor(uint8_t color)
	{
		return bg_palette[color];
	}

	void drawBGTileAt(tile * tile, uint8_t x, uint8_t y)
	{
		for (uint8_t _y = 0; _y < 8; _y++)
		{
			for (uint8_t _x = 0; _x < 8; _x++)
			{
				uint8_t index = _x + 8 * _y;
				uint8_t color = getBackgroundColor((*tile)[index]);
				drawPixel(x, y, color);
			}
		}
	}

	void drawBG()
	{
		for (uint8_t _y = 0; _y < 32; _y++)
		{
			for (uint8_t _x = 0; _x < 32; _x++)
			{
				uint8_t index = _x + 32 * _y;
				uint8_t tileIndex = *(ram.memory + 0x9800);
				tile * tile = getTile(tileIndex);
				drawBGTileAt(tile, _x * 8, _y * 8);
				if (_x * 8 > 144)
					continue;
			}
			if (_y * 8 > 160)
				continue;
		}
	}
	
public:
	union {
		uint8_t control;
#pragma pack(push, 1)
		struct {
			uint8_t bg_display : 1;
			uint8_t obj_dispay_enable : 1;
			uint8_t obj_size : 1; // (0 = 8x8, 1 = 8x16)
			uint8_t bg_tilemap_select : 1;
			uint8_t bg_win_tile_data_select : 1;
			uint8_t win_display_enable : 1;
			uint8_t win_tilemap_select : 1;
			uint8_t lcd_display_enable : 1;
		};
#pragma pack(pop)
	};
	uint8_t scrollX, scrollY;
	uint8_t winPosX, winPosY;
	struct palette
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t shade0 : 2;
				uint8_t shade1 : 2;
				uint8_t shade2 : 2;
				uint8_t shade3 : 2;
			};
#pragma pack(pop)
		};

		palette& operator=(uint8_t value) { this->value = value; return *this; }
		operator uint8_t() const { return this->value; }
		uint8_t operator[](uint8_t shadeIndex)
		{
			uint8_t shift = 6 - 2 * shadeIndex;
			return (value & (0x3 << shift)) >> shift;
		}
	};
	palette bg_palette, obj_palette0, obj_palette1;

	Display(RAM & ram) : ram(ram)
	{
		startTime = std::chrono::system_clock::now();
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			throw std::runtime_error("SDL: " + std::string(SDL_GetError()));
		}
		win = SDL_CreateWindow("GBEmu", 100, 100, 160 * 4, 144 * 4, SDL_WINDOW_SHOWN);
		if (win == nullptr)
		{
			std::string msg = "SDL: " + std::string(SDL_GetError());
			SDL_Quit();
			throw std::runtime_error(msg);
		}
		renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (renderer == nullptr)
		{
			std::string msg = "SDL: " + std::string(SDL_GetError());
			SDL_Quit();
			throw std::runtime_error(msg);
		}
	}

	~Display()
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(win);
		SDL_Quit();
	}

	void update();

	uint8_t LY()
	{
		//return (getMicroseconds() / 109) % 154;
		return 145;
	}
};

