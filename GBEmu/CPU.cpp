#include "stdafx.h"
#include "CPU.h"

static const double delay_2ms = 0.238418579 * 4196 * 4; // (4196 cycles ~= 1ms)

void CPU::run()
{
	std::cout << "--- Starting execution ---" << std::endl;
	auto last_sleep = std::chrono::system_clock::now();
	while (running)
	{
		if (!halted)
			step();
		else
			cycleWait(4);
		display.update();
		checkInterrupts();
		std::this_thread::yield();
		int64_t remainder = cycleCount - 4 * 4196;
		if (remainder > 0) // Amount of cycles in 4 ms
		{
			cycleCount = remainder;
			auto now = std::chrono::system_clock::now();
			auto computation_time = now - last_sleep;
			auto delay = std::chrono::milliseconds(4) - computation_time;
			last_sleep = now;
			std::this_thread::sleep_for(delay);
			updateInput();
		}
	}
	std::cout << "--- Execution stopped ---" << std::endl;
	regs.dump();
}

void CPU::DMATranfer(uint8_t source)
{
	static const uint16_t destinationAddress = 0xFE00;
	uint16_t sourceAddress = source << 8;
	for (uint8_t index = 0; index < 0xA0; index++)
		ram[destinationAddress + index] = (uint8_t)ram[sourceAddress + index];
}


void CPU::updateInput()
{
	SDL_GameControllerUpdate();
	display.controller_status.button_a = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_B);
	display.controller_status.button_b = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_A);
	display.controller_status.button_start = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_START);
	display.controller_status.button_select = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_BACK);
	display.controller_status.up = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
	display.controller_status.down = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	display.controller_status.left = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	display.controller_status.right = SDL_GameControllerGetButton(display.controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
}
