#include "stdafx.h"
#include "CPU.h"

static const double delay_1ms = 0.238418579 * 4196; // (microseconds for 4196 cycles ~= 1ms)

void CPU::run()
{
	std::cout << "--- Starting execution ---" << std::endl;
	uint64_t counter = 0;
	auto last_sleep = std::chrono::system_clock::now();
	while (running)
	{
		if (!halted)
			step();
		else
			cycleWait(4);
		display.update();
		timer.update();
		checkInterrupts();
		std::this_thread::yield();
		counter++;
		if (counter == 4196) // Amount of cycles in 1 ms
		{
			auto now = std::chrono::system_clock::now();
			auto computation_time = now - last_sleep;
			auto delay = std::chrono::milliseconds(1) - computation_time;
			last_sleep = now;
			std::this_thread::sleep_for(delay);
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
