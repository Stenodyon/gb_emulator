#pragma once

#include <SDL.h>
#include <chrono>
#include <thread>


class Display
{
private:
	SDL_Window * win;
	SDL_Renderer * renderer;

	std::chrono::time_point<std::chrono::system_clock> startTime;
	uint64_t getMicroseconds()
	{
		auto diff = std::chrono::system_clock::now() - startTime;
		return std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
	}
	
public:
	uint8_t scrollX, scrollY;

	Display()
	{
		startTime = std::chrono::system_clock::now();
		if (SDL_Init(SDL_INIT_VIDEO) != 0)
		{
			throw std::runtime_error("SDL: " + std::string(SDL_GetError()));
		}
		win = SDL_CreateWindow("GBEmu", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
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

	uint8_t LY()
	{
		return (getMicroseconds() / 109) % 154;
	}
};

