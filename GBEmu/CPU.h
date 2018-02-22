#pragma once

#include <stdexcept>

#include "stdafx.h"
#include "Program.h"
#include "registers.h"

class OpcodeNotImplemented : public std::runtime_error
{
public:
	OpcodeNotImplemented(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class CPU
{
private:
	Regs registers;
	Program program;
	bool running = true;
	bool interruptsEnabled = true;

public:
	CPU(uint8_t * program_data) : program(program_data)
	{
		program.jump(0x100); // Entry point is 0x100
		std::cout << "CPU initialized" << std::endl;
	}

	~CPU() {}

	void step()
	{
		static uint8_t counter = 0;
		if (counter >= 100)
			running = false;
		counter++;

		uint16_t currentPointer = program.getPointer();
		uint8_t instr = program.nextB();
		switch (instr)
		{
		case 0x00: // NOP
			break;
		case 0xC3: // JP a16
		{
			uint16_t jumpAddress = program.nextW();
			program.jump(jumpAddress);
			break;
		}
		case 0xF3: // DI Disable Interrupts
		{
			interruptsEnabled = false;
			break;
		}
		default:
		{
			std::ostringstream sstream;
			sstream << "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << +instr << std::nouppercase;
			sstream << " at address 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << +currentPointer;
			throw OpcodeNotImplemented(sstream.str());
		}
		}
	}

	void run()
	{
		while(running)
			step();
	}
};

