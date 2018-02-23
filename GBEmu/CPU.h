#pragma once

#include <stdexcept>
#include <cstring>

#include "stdafx.h"
#include "registers.h"
#include "RAM.h"

class OpcodeNotImplemented : public std::runtime_error
{
public:
	OpcodeNotImplemented(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class CPU
{
private:
	Regs regs;
	RAM ram;
	bool running = true;
	bool interruptsEnabled = true;

public:
	CPU(uint8_t * program_data, size_t size) : ram(program_data, size)
	{
		jump(0x100); // Entry point is 0x100

		regs.AF = 0x01B0;
		regs.BC = 0x0013;
		regs.DE = 0x00D8;
		regs.HL = 0x014D;
		regs.SP = 0xFFFE;

		ram.memory[0xFF05] = 0x00;
		ram.memory[0xFF06] = 0x00;
		ram.memory[0xFF07] = 0x00;
		ram.memory[0xFF10] = 0x80;
		ram.memory[0xFF11] = 0xBF;
		ram.memory[0xFF12] = 0xF3;
		ram.memory[0xFF14] = 0xBF;
		ram.memory[0xFF16] = 0x3F;
		ram.memory[0xFF17] = 0x00;
		ram.memory[0xFF19] = 0xBF;
		ram.memory[0xFF1A] = 0x7F;
		ram.memory[0xFF1B] = 0xFF;
		ram.memory[0xFF1C] = 0x9F;
		ram.memory[0xFF1E] = 0xBF;
		ram.memory[0xFF20] = 0xFF;
		ram.memory[0xFF21] = 0x00;
		ram.memory[0xFF22] = 0x00;
		ram.memory[0xFF23] = 0xBF;
		ram.memory[0xFF24] = 0x77;
		ram.memory[0xFF25] = 0xF3;
		ram.memory[0xFF26] = 0xF1;
		ram.memory[0xFF40] = 0x91;
		ram.memory[0xFF42] = 0x00;
		ram.memory[0xFF43] = 0x00;
		ram.memory[0xFF45] = 0x00;
		ram.memory[0xFF47] = 0xFC;
		ram.memory[0xFF48] = 0xFF;
		ram.memory[0xFF49] = 0xFF;
		ram.memory[0xFF4A] = 0x00;
		ram.memory[0xFF4B] = 0x00;
		ram.memory[0xFFFF] = 0x00;

		std::cout << "CPU initialized" << std::endl;
	}

	~CPU() {}

	uint8_t nextB()
	{
		uint8_t value = ram[regs.PC];
		regs.PC++;
		return value;
	}

	uint16_t nextW()
	{
		uint16_t value = ram[regs.PC];
		regs.PC += 2;
		return value;
	}

	void jump(uint16_t address)
	{
		std::cout << "Jump to " << hex<uint16_t>(address) << std::endl;
		regs.PC = address;
	}

	void jumpRelative(uint8_t jump)
	{
		std::cout << "Relative jump of " << hex<int8_t>(jump) << std::endl;
		regs.PC += (int8_t)jump;
	}

	void push(uint16_t value)
	{
		(uint16_t&)ram[regs.SP - 1] = value;
		regs.SP -= 2;
	}

	uint16_t pop()
	{
		regs.SP += 2;
		return ram[regs.SP - 1];
	}

	void step()
	{
		static uint64_t counter = 0;
		if (counter >= 1000)
			running = false;
		counter++;

		uint16_t currentPointer = regs.PC;
		uint8_t instr = nextB();
		switch (instr)
		{
		case 0x00: // NOP
			break;
		case 0x20: // JR NZ, d8
		{
			uint8_t jump = nextB();
			if (!regs.Zf)
				jumpRelative(jump);
			break;
		}
		case 0x3E: // LD A, d8
		{
			int8_t value = nextB();
			regs.A = value;
			break;
		}
		case 0x47: // LD B, A
		{
			regs.B = regs.A;
			break;
		}
		case 0xAF: // XOR A
		{
			regs.A ^= regs.A;
			regs.Zf = regs.Nf = regs.Hf = regs.Cf = 0;
			break;
		}
		case 0xC3: // JP a16
		{
			uint16_t jumpAddress = nextW();
			jump(jumpAddress);
			break;
		}
		case 0xCB: // Prefix CB
		{
			prefixCB();
			break;
		}
		case 0xCD: // CALL a16
		{
			uint16_t jumpAddress = nextW();
			uint16_t returnAddress = regs.PC;
			push(returnAddress);
			jump(jumpAddress);
			break;
		}
		case 0xE0: // LDH (a8), A
		{
			uint8_t nextVal = nextB();
			uint16_t value = 0xFF00 + nextVal;
			if (value >= 0xFF00 && value <= 0xFF7F)
				OnIOWrite(nextVal, regs.A);
			ram[value] = regs.A;
			break;
		}
		case 0xF0: // LDH A, (a8)
		{
			uint8_t nextVal = nextB();
			uint16_t value = 0xFF00 + nextVal;
			if (value >= 0xFF00 && value <= 0xFF7F)
				ram[value] = OnIORead(nextVal);
			regs.A = ram[value];
			break;
		}
		case 0xF3: // DI Disable Interrupts
		{
			interruptsEnabled = false;
			break;
		}
		case 0xFE: // CP d8
		{
			uint8_t value = nextB();
			regs.Zf = regs.A == value;
			regs.Nf = 1;
			regs.Hf = halfcarry(regs.A, value);
			regs.Cf = regs.A < value;
			break;
		}
		case 0xFF: // RST 0x38 (equivalent to CALL 0x38)
		{
			uint16_t returnAddress = nextW();
			push(returnAddress);
			jump(0x0038);
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

	void prefixCB()
	{
		uint16_t currentPointer = regs.PC;
		uint8_t instr = nextB();
		switch (instr)
		{
		case 0x87: // RES 0,A
		{
			regs.A = 0;
			break;
		}
		default:
		{
			std::ostringstream sstream;
			sstream << "0xCB 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << +instr << std::nouppercase;
			sstream << " at address 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << +currentPointer;
			throw OpcodeNotImplemented(sstream.str());
		}
		}
	}

	void run()
	{
		std::cout << "--- Starting execution ---" << std::endl;
		while(running)
			step();
		std::cout << "--- Executed all instructions ---" << std::endl;
	}

	void OnIOWrite(uint8_t port, uint8_t value)
	{
		std::cout << "Wrote 0x" << std::uppercase << std::setfill('0') << std::setw(2) << +value << std::nouppercase;
		std::cout << " to IO port 0x" << std::uppercase << std::setfill('0') << std::setw(2) << +port << std::nouppercase << std::endl;
	}

	uint8_t OnIORead(uint8_t port)
	{
		std::cout << "Read from IO port 0x" << std::uppercase << std::setfill('0') << std::setw(2) << +port << std::nouppercase << std::endl;
		return 0;
	}

	bool halfcarry(uint8_t a, uint8_t b)
	{
		return (((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10;
	}
	
	bool halfcarry(uint16_t a, uint16_t b)
	{
		return (((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10;
	}
};

