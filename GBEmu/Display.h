#pragma once

#include <SDL.h>
#include <chrono>
#include <thread>

#include "RAM.h"
#include "Joypad.h"

struct tile
{
	uint8_t data[16];

	uint8_t operator[](uint8_t pixelIndex)
	{
		uint8_t index = (pixelIndex >> 3) * 2;
		uint8_t bitIndex = 0x7 - (pixelIndex & 0x7);
		uint8_t mask = 0x01 << bitIndex;
		bool lower = data[index] & mask;
		bool upper = data[index + 1] & mask;
		return (upper ? 0x2 : 0x0) | (lower ? 0x1 : 0x0);
	}
};
static_assert(sizeof(tile) == 16, "tile structure is not 16 bytes");

struct sprite
{
	uint8_t y;
	uint8_t x;
	uint8_t chr_code;
	
	union {
		uint8_t attributes;
#pragma pack(push, 1)
		struct {
			uint8_t color_palette : 3; // CGB Only
			uint8_t char_bank : 1; // CGB Only
			uint8_t palette : 1;
			uint8_t hor_flip : 1;
			uint8_t vert_flip : 1;
			uint8_t priority : 1;
		};
#pragma pack(pop)
	};
};
static_assert(sizeof(sprite) == 4, "sprite structure is not 4 bytes");

static const double OAM_us = 19.07348632;
static const double HBLANK_us = 60.081481908;
static const double LINE_us = 108.718872024;
static const double VBLANK_us = 15655.5175715;
static const double REFRESH_us = 16742.7062917;

class Display
{
public:
	pad_status keyboard_status;
	pad_status controller_status;
	SDL_GameController * controller;

	uint8_t vram[0x2000];
	uint8_t oam_ram[0xA0];

private:
	SDL_Window * win;
	SDL_Renderer * renderer;
	SDL_Texture * bg_texture;
	CPU * cpu;
	double lastElapsedTime = 0;
	double elapsedTime = 0;

public:
	union pixel
	{
		uint8_t data[4];
		struct {
			uint8_t a;
			uint8_t b;
			uint8_t g;
			uint8_t r;
		};
	};
	static_assert(sizeof(pixel) == 4, "pixel struct is not 4 bytes");

private:
	pixel * pixel_buffer;
	SDL_Texture * win_texture;
	uint64_t frameCycles = 0;

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
			uint8_t shift = 2 * shadeIndex;
			return (value & (0x3 << shift)) >> shift;
		}
	};

	void incrementFrameCycles();
	void drawPixel(uint8_t x, uint8_t y, uint8_t color, bool transparency = false);
	uint8_t getBGColor(uint8_t x, uint8_t y);
	uint8_t getWindowColor(uint8_t x, uint8_t y);
	uint8_t getBGColorUnderPixel(uint8_t x, uint8_t y);
	uint8_t getWinColorUnderPixel(uint8_t x, uint8_t y);
	uint8_t getSpriteColor(sprite * sprite, uint8_t x, uint8_t y);
	tile * getSpriteTile(uint8_t index);
	tile * getTile(uint8_t index);
	void getVisibleSprites(uint8_t line, sprite * buffer[], uint8_t &count);
	void drawLine(uint8_t line);

	//TODO: Implement the Window
	
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
	struct _lcd_status
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t stat_mode : 2;
				uint8_t ly_coincidence : 1;
				uint8_t hblank_int : 1;
				uint8_t vblank_int : 1;
				uint8_t oam_int : 1;
				uint8_t coincidence_int : 1;
			};
#pragma pack(pop)
		};

		_lcd_status& operator=(uint8_t value)
		{
			this->value = value & 0x78; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	_lcd_status status;
	uint8_t scrollX, scrollY;
	uint8_t winPosX, winPosY;
	uint8_t ly, lyc;
	palette bg_palette, obj_palette0, obj_palette1;

	Display(CPU * cpu) : cpu(cpu)
	{
		pixel_buffer = (pixel*)malloc(160 * 144 * sizeof(pixel));
		std::memset(pixel_buffer, 0, 160 * 144 * sizeof(pixel));
		if (pixel_buffer == nullptr)
		{
			std::cerr << "Could not allocate pixel buffer" << std::endl;
		}
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) != 0)
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
		renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == nullptr)
		{
			std::string msg = "SDL: " + std::string(SDL_GetError());
			SDL_Quit();
			throw std::runtime_error(msg);
		}
		win_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
		if (win_texture == nullptr)
		{
			std::string msg = "SDL: " + std::string(SDL_GetError());
			SDL_Quit();
			throw std::runtime_error(msg);
		}
		if (SDL_NumJoysticks() > 0)
		{
			std::cout << "Controller found" << std::endl;
			SDL_JoystickEventState(SDL_IGNORE);
			controller = SDL_GameControllerOpen(0);
			if (controller == nullptr)
			{
				std::cout << "Could not open the controller :(" << std::endl;
			}
		}
		else
		{
			std::cout << "No joysticks found" << std::endl;
		}
	}

	~Display()
	{
		free(pixel_buffer);
		if(controller != nullptr)
			SDL_GameControllerClose(controller);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(win);
		SDL_Quit();
	}

	void update();
	void OnMachineCycle(uint64_t cycles);
};

