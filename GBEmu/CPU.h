#pragma once

#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>

#include "stdafx.h"
#include "registers.h"
#include "RAM.h"
#include "Display.h"

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
	Display display;
	bool running = true;
	bool interruptsEnabled = true;

	struct _int_flag
	{
		union
		{
			uint8_t value;
#pragma pack(push, 1)
			struct
			{
				uint8_t vblank : 1;
				uint8_t lcd_stat : 1;
				uint8_t timer : 1;
				uint8_t serial : 1;
				uint8_t joypad : 1;
			};
#pragma pack(pop)
		};

		_int_flag& operator=(uint8_t value) { this->value = value; return *this; }
		operator uint8_t() const { return this->value; }
	};
	_int_flag int_flag;

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

	void cycleWait(uint64_t cycleCount)
	{
		uint64_t microseconds = (uint64_t)(cycleCount * 0.23866);
		std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
	}

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
		ram[regs.SP - 1] = value;
		regs.SP -= 2;
	}

	uint16_t pop()
	{
		regs.SP += 2;
		return ram[regs.SP - 1];
	}

	void step()
	{
		/*
		static uint64_t counter = 0;
		if (counter >= 1000)
			running = false;
		counter++;
		//*/

		uint16_t currentPointer = regs.PC;
		uint8_t instr = nextB();
		std::cout << "[" << hex<uint16_t>(currentPointer) << "] " << hex<uint8_t>(instr) << std::endl;
		switch (instr)
		{
		case 0x00: // NOP
			cycleWait(4);
			break;
		case 0x01: // LD BC, d16
		{
			uint16_t value = nextW();
			regs.BC = value;
			cycleWait(12);
			break;
		}
		case 0x0B: // DEC BC
		{
			regs.BC--;
			cycleWait(8);
			break;
		}
		case 0x20: // JR NZ, r8
		{
			regs.dump();
			uint8_t jump = nextB();
			if (!regs.Zf)
			{
				jumpRelative(jump);
				cycleWait(12);
			}
			else
			{
				cycleWait(8);
			}
			break;
		}
		case 0x21: // LD HL, d16
		{
			uint16_t value = nextW();
			regs.HL = value;
			cycleWait(12);
			break;
		}
		case 0x23: // INC HL
		{
			regs.HL++;
			cycleWait(8);
			break;
		}
		case 0x31: // LD SP, d16
		{
			uint16_t value = nextW();
			regs.SP = value;
			cycleWait(12);
			break;
		}
		case 0x36: // LD (HL), d8
		{
			uint8_t value = nextB();
			ram[regs.HL] = value;
			cycleWait(12);
			break;
		}
		case 0x3E: // LD A, d8
		{
			int8_t value = nextB();
			regs.A = value;
			cycleWait(8);
			break;
		}
		case 0x47: // LD B, A
		{
			regs.B = regs.A;
			cycleWait(4);
			break;
		}
		case 0x78: // LD A, B
		{
			regs.A = regs.B;
			cycleWait(4);
			break;
		}
		case 0xAF: // XOR A
		{
			regs.A ^= regs.A;
			regs.Zf = regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB1: // OR C
		{
			regs.A |= regs.C;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xC3: // JP a16
		{
			uint16_t jumpAddress = nextW();
			jump(jumpAddress);
			cycleWait(16);
			break;
		}
		case 0xC9: // RET
		{
			uint16_t returnAddress = pop();
			jump(returnAddress);
			cycleWait(16);
			break;
		}
		case 0xCB: // Prefix CB
		{
			cycleWait(4);
			prefixCB();
			break;
		}
		case 0xCD: // CALL a16
		{
			uint16_t jumpAddress = nextW();
			uint16_t returnAddress = regs.PC;
			push(returnAddress);
			jump(jumpAddress);
			cycleWait(24);
			break;
		}
		case 0xE0: // LDH (a8), A
		{
			uint8_t nextVal = nextB();
			uint16_t value = 0xFF00 + nextVal;
			if (value >= 0xFF00 && value <= 0xFF7F)
				OnIOWrite(nextVal, regs.A);
			ram[value] = regs.A;
			cycleWait(12);
			break;
		}
		case 0xE6: // AND d8
		{
			uint8_t value = nextB();
			regs.A &= value;
			regs.Zf = regs.A == 0;
			regs.Nf = 0;
			regs.Hf = 1;
			regs.Cf = 0;
			cycleWait(8);
			break;
		}
		case 0xF0: // LDH A, (a8)
		{
			uint8_t nextVal = nextB();
			uint16_t value = 0xFF00 + nextVal;
			if (value >= 0xFF00 && value <= 0xFF7F)
				regs.A = OnIORead(nextVal);
			else
				regs.A = ram[value];
			cycleWait(12);
			break;
		}
		case 0xF3: // DI Disable Interrupts
		{
			interruptsEnabled = false;
			cycleWait(4);
			break;
		}
		case 0xFE: // CP d8
		{
			uint8_t value = nextB();
			regs.Zf = regs.A == value;
			regs.Nf = 1;
			regs.Hf = halfcarry(regs.A, value);
			regs.Cf = regs.A < value;
			cycleWait(8);
			break;
		}
		case 0xFF: // RST 0x38 (equivalent to CALL 0x38)
		{
			uint16_t returnAddress = nextW();
			push(returnAddress);
			jump(0x0038);
			cycleWait(16);
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
			cycleWait(8);
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
		while (running)
			step();
		std::cout << "--- Executed all instructions ---" << std::endl;
		regs.dump();
	}

	void OnIOWrite(uint8_t port, uint8_t value)
	{
		switch (port)
		{
		case 0x0F: // Interrupt flag
		{
			int_flag = value;
			break;
		}
		case 0x42: // Display - Scroll Y
		{
			display.scrollY = value;
			break;
		}
		case 0x43: // Display - Scroll X
		{
			display.scrollX = value;
			break;
		}
		default:
			std::cout << "Warning, writing " << hex<uint8_t>(value) << " to IO port " << hex<uint8_t>(port) << std::endl;
			throw std::runtime_error("IO");
			break;
		}
	}

	uint8_t OnIORead(uint8_t port)
	{
		switch (port)
		{
		case 0x0F: // Interrupt flag
		{
			return int_flag;
		}
		case 0x42: // Display - Scroll Y
		{
			return display.scrollY;
		}
		case 0x43: // Display - Scroll X
		{
			return display.scrollX;
		}
		case 0x44: // LCD - LY
		{
			uint8_t ly = display.LY();
			std::cout << "Read LY: " << hex<uint8_t>(ly) << std::endl;
			return ly;
		}
		default:
			std::cout << "Warning, reading from unimplemented IO port " << hex<uint8_t>(port) << std::endl;
			throw std::runtime_error("IO");
			break;
		}
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

