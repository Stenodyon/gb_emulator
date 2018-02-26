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

