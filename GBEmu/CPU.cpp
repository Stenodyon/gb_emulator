#include "stdafx.h"
#include "CPU.h"

void CPU::run()
{
	std::cout << "--- Starting execution ---" << std::endl;
	while (running)
	{
		if (!halted)
			step();
		else
			cycleWait(4);
		display.update();
		timer.update();
		checkInterrupts();
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::this_thread::yield();
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
