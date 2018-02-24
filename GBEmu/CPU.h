#pragma once

#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

#include "stdafx.h"
#include "registers.h"
#include "RAM.h"
#include "Display.h"
#include "Serial.h"
#include "Timer.h"
#include "Sound.h"
#include "Joypad.h"

class OpcodeNotImplemented : public std::runtime_error
{
public:
	OpcodeNotImplemented(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class IOPortNotImplemented : public std::runtime_error
{
public:
	IOPortNotImplemented(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class CPU
{
public:
	std::vector<uint16_t> breakpoints;

	uint64_t cycleCount = 0;

private:
	Regs regs;
	RAM ram;
	Display display;
	Serial serial;
	Timer timer;
	Sound sound;
	Joypad joypad;
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
	_int_flag int_enable, int_flag;

public:
	CPU(uint8_t * program_data, size_t size) : ram(program_data, size, this) , display(ram), timer(this)
	{
		jump(0x100); // Entry point is 0x100

		regs.AF = 0x01B0;
		regs.BC = 0x0013;
		regs.DE = 0x00D8;
		regs.HL = 0x014D;
		regs.SP = 0xFFFE;

		ram[0xFF05] = (uint8_t)0x00;
		ram[0xFF06] = (uint8_t)0x00;
		ram[0xFF07] = (uint8_t)0x00;
		ram[0xFF10] = (uint8_t)0x80;
		ram[0xFF11] = (uint8_t)0xBF;
		ram[0xFF12] = (uint8_t)0xF3;
		ram[0xFF14] = (uint8_t)0xBF;
		ram[0xFF16] = (uint8_t)0x3F;
		ram[0xFF17] = (uint8_t)0x00;
		ram[0xFF19] = (uint8_t)0xBF;
		ram[0xFF1A] = (uint8_t)0x7F;
		ram[0xFF1B] = (uint8_t)0xFF;
		ram[0xFF1C] = (uint8_t)0x9F;
		ram[0xFF1E] = (uint8_t)0xBF;
		ram[0xFF20] = (uint8_t)0xFF;
		ram[0xFF21] = (uint8_t)0x00;
		ram[0xFF22] = (uint8_t)0x00;
		ram[0xFF23] = (uint8_t)0xBF;
		ram[0xFF24] = (uint8_t)0x77;
		ram[0xFF25] = (uint8_t)0xF3;
		ram[0xFF26] = (uint8_t)0xF1;
		ram[0xFF40] = (uint8_t)0x91;
		ram[0xFF42] = (uint8_t)0x00;
		ram[0xFF43] = (uint8_t)0x00;
		ram[0xFF45] = (uint8_t)0x00;
		ram[0xFF47] = (uint8_t)0xFC;
		ram[0xFF48] = (uint8_t)0xFF;
		ram[0xFF49] = (uint8_t)0xFF;
		ram[0xFF4A] = (uint8_t)0x00;
		ram[0xFF4B] = (uint8_t)0x00;
		ram[0xFFFF] = (uint8_t)0x00;

		std::cout << "CPU initialized" << std::endl;
	}

	~CPU() {}

	void cycleWait(uint64_t cycleCount)
	{
		uint64_t microseconds = (uint64_t)(cycleCount * 0.23866);
		//std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
		this->cycleCount += cycleCount;
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
#ifdef _DEBUG
		std::cout << "Jump to " << hex<uint16_t>(address) << std::endl;
#endif
		regs.PC = address;
	}

	void jumpRelative(uint8_t jump)
	{
		uint16_t address = regs.PC + (int8_t)jump;
		regs.PC = address;
#ifdef _DEBUG
		std::cout << "Relative jump to " << hex<uint16_t>(address) << std::endl;
#endif
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
		if (std::find(breakpoints.begin(), breakpoints.end(), currentPointer) != breakpoints.end())
		{
			std::cout << "Breakpoint " << hex<uint16_t>(currentPointer) << std::endl;
			regs.dump();
			getchar();
		}
		uint8_t instr = nextB();
#ifdef _DEBUG
		std::cout << "[" << hex<uint16_t>(currentPointer) << "] " << hex<uint8_t>(instr) << " ";
#endif
		switch (instr)
		{
		case 0x00: // NOP
#ifdef _DEBUG
			std::cout << "NOP" << std::endl;
#endif
			cycleWait(4);
			break;
		case 0x01: // LD BC, d16
		{
			uint16_t value = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(value) << " LD BC, " << +value << std::endl;
#endif
			regs.BC = value;
			cycleWait(12);
			break;
		}
		case 0x03: // INC BC
		{
#ifdef _DEBUG
			std::cout << "INC BC" << std::endl;
#endif
			regs.BC++;
			cycleWait(8);
			break;
		}
		case 0x05: // DEC B
		{
#ifdef _DEBUG
			std::cout << "DEC B" << std::endl;
#endif
			regs.Hf = halfcarry(regs.B, 0xFF);
			regs.B--;
			regs.Zf = regs.B == 0;
			regs.Nf = 1;
			cycleWait(4);
			break;
		}
		case 0x06: // LD B, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " LD B, " << +value << std::endl;
#endif
			regs.B = value;
			cycleWait(8);
			break;
		}
		case 0x0B: // DEC BC
		{
#ifdef _DEBUG
			std::cout << "DEC BC" << std::endl;
#endif
			regs.BC--;
			cycleWait(8);
			break;
		}
		case 0x0C: // INC C (with flags)
		{
#ifdef _DEBUG
			std::cout << "INC C" << std::endl;
#endif
			regs.Hf = halfcarry(regs.C, 0x1);
			regs.C++;
			regs.Zf = regs.C == 0x00;
			regs.Nf = 1;
			cycleWait(3);
			break;
		}
		case 0x0E: // LD C, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " LD C, " << +value << std::endl;
#endif
			regs.C = value;
			cycleWait(8);
			break;
		}
		case 0x11: // LD DE, d16
		{
			uint16_t value = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(value) << " LD DE, " << +value << std::endl;
#endif
			regs.DE = value;
			cycleWait(12);
			break;
		}
		case 0x13: // INC DE
		{
#ifdef _DEBUG
			std::cout << "INC DE" << std::endl;
#endif
			regs.DE++;
			cycleWait(8);
			break;
		}
		case 0x15: // DEC D
		{
#ifdef _DEBUG
			std::cout << "DEC D" << std::endl;
#endif
			regs.Hf = halfcarry(regs.D, 0xFF);
			regs.D--;
			regs.Zf = regs.D == 0;
			regs.Nf = 1;
			cycleWait(4);
			break;
		}
		case 0x16: // LD D, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " LD D, " << +value << std::endl;
#endif
			regs.D = value;
			cycleWait(8);
			break;
		}
		case 0x18: // JR r8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " JR " << +(int8_t)value << std::endl;
#endif
			jumpRelative(value);
			cycleWait(12);
			break;
		}
		case 0x19: // ADD HL, DE
		{
#ifdef _DEBUG
			std::cout << "ADD HL, DE" << std::endl;
#endif
			regs.Hf = halfcarry(regs.HL, regs.DE);
			regs.Cf = ((uint64_t)regs.HL * regs.DE) > 0xFFFF;
			regs.HL += regs.DE;
			regs.Nf = 0;
			cycleWait(8);
			break;
		}
		case 0x1A: // LD A, (DE)
		{
#ifdef _DEBUG
			std::cout << "LD A, (DE)" << std::endl;
#endif
			uint8_t value = ram[regs.DE];
			regs.A = value;
			cycleWait(8);
			break;
		}
		case 0x1B: // DEC DE
		{
#ifdef _DEBUG
			std::cout << "DEC DE" << std::endl;
#endif
			regs.DE--;
			cycleWait(8);
			break;
		}
		case 0x1D: // DEC E
		{
#ifdef _DEBUG
			std::cout << "DEC E" << std::endl;
#endif
			regs.Hf = halfcarry(regs.E, 0xFF);
			regs.E--;
			regs.Zf = regs.E == 0;
			regs.Nf = 1;
			cycleWait(4);
			break;
		}
		case 0x1E: // LD E, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << "LD E, " << +value << std::endl;
#endif
			regs.E = value;
			cycleWait(8);
			break;
		}
		case 0x1F: // RRA (= RR A)
		{
#ifdef _DEBUG
			std::cout << "RRA" << std::endl;
#endif
			RR(regs.A);
			regs.Zf = 0;
			cycleWait(4);
			break;
		}
		case 0x20: // JR NZ, r8
		{
			//regs.dump();
			uint8_t jump = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(jump) << " JR NZ, " << +(int8_t)jump << std::endl;
#endif
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
#ifdef _DEBUG
			std::cout << hex<uint16_t>(value) << " LD HL, " << +value << std::endl;
#endif
			regs.HL = value;
			cycleWait(12);
			break;
		}
		case 0x22: // LD (HL+), A (puts A into (HL) and increment HL)
		{
#ifdef _DEBUG
			std::cout << "LD (HL+), A" << std::endl;
#endif
			ram[regs.HL] = regs.A;
			regs.HL++;
			cycleWait(8);
			break;
		}
		case 0x23: // INC HL
		{
#ifdef _DEBUG
			std::cout << "INC HL" << std::endl;
#endif
			regs.HL++;
			cycleWait(8);
			break;
		}
		case 0x24: // INC H
		{
#ifdef _DEBUG
			std::cout << "INC H" << std::endl;
#endif
			regs.Hf = halfcarry(regs.H, 0x01);
			regs.H++;
			regs.Zf = regs.H == 0;
			regs.Nf = 0;
			cycleWait(4);
			break;
		}
		case 0x25: // DEC H
		{
#ifdef _DEBUG
			std::cout << "DEC H" << std::endl;
#endif
			regs.Hf = halfcarry(regs.H, 0xFF);
			regs.H--;
			regs.Zf = regs.H == 0;
			regs.Nf = 1;
			cycleWait(4);
			break;
		}
		case 0x26: // LD H, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " LD H, " << +value << std::endl;
#endif
			regs.H = value;
			cycleWait(8);
			break;
		}
		case 0x28: // JR Z, r8
		{
			uint8_t jump = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(jump) << " JR Z, " << +(int8_t)jump << std::endl;
#endif
			if (regs.Zf)
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
		case 0x29: // ADD HL, HL
		{
#ifdef _DEBUG
			std::cout << "ADD HL, HL" << std::endl;
#endif
			regs.Hf = halfcarry(regs.HL, regs.HL);
			regs.Cf = ((uint64_t)regs.HL * 2) > 0xFFFF;
			regs.HL += regs.HL;
			regs.Nf = 0;
			cycleWait(8);
			break;
		}
		case 0x2A: // LD A, (HL+) (puts (HL) intro A and increment HL)
		{
#ifdef _DEBUG
			std::cout << "LD A, (HL+)" << std::endl;
#endif
			regs.A = ram[regs.HL];
			regs.HL++;
			cycleWait(8);
			break;
		}
		case 0x2C: // INC L
		{
			regs.Hf = halfcarry(regs.L, 0x01);
			regs.L++;
			regs.Zf = regs.L == 0;
			regs.Nf = 1;
			cycleWait(4);
#ifdef _DEBUG
			std::cout << "INC L (" << hex<uint8_t>(regs.L) << ") Z flag: " << (bool)(regs.Zf) << std::endl;
#endif
			break;
		}
		case 0x2D: // DEC L
		{
#ifdef _DEBUG
			std::cout << "DEC L" << std::endl;
#endif
			regs.Hf = halfcarry(regs.L, 0xFF);
			regs.L--;
			regs.Zf = regs.L == 0;
			regs.Nf = 1;
			cycleWait(4);
			break;
		}
		case 0x30: // JR NC, r8
		{
			//regs.dump();
			uint8_t jump = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(jump) << " JR NC, " << +(int8_t)jump << std::endl;
#endif
			if (!regs.Cf)
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
		case 0x31: // LD SP, d16
		{
			uint16_t value = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(value) << " LD SP, " << +value << std::endl;
#endif
			regs.SP = value;
			cycleWait(12);
			break;
		}
		case 0x32: // LD (HL-), A
		{
#ifdef _DEBUG
			std::cout << "LD (HL-), A" << std::endl;
#endif
			ram[regs.HL] = regs.A;
			regs.HL--;
			cycleWait(8);
			break;
		}
		case 0x35: // DEC (HL)
		{
#ifdef _DEBUG
			std::cout << "DEC (HL)" << std::endl;
#endif
			uint8_t value = ram[regs.HL];
			regs.Hf = halfcarry(value, 0xFF);
			value--;
			regs.Zf = value == 0;
			regs.Nf = 1;
			ram[regs.HL] = value;
			cycleWait(12);
			break;
		}
		case 0x36: // LD (HL), d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(value) << " LD (HL), " << +value << std::endl;
#endif
			ram[regs.HL] = value;
			cycleWait(12);
			break;
		}
		case 0x3D: // DEC A
		{
#ifdef _DEBUG
			std::cout << "DEC A" << std::endl;
#endif
			regs.Hf = halfcarry(regs.A, 0xFF);
			regs.A--;
			regs.Zf = regs.A == 0;
			regs.Nf = 1;
			cycleWait(4);
			break;
		}
		case 0x3E: // LD A, d8
		{
			int8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " LD A, " << +value << std::endl;
#endif
			regs.A = value;
			cycleWait(8);
			break;
		}
		case 0x42: // LD B, D
		{
#ifdef _DEBUG
			std::cout << "LD B, D" << std::endl;
#endif
			regs.B = regs.D;
			cycleWait(4);
			break;
		}
		case 0x46: // LD B, (HL)
		{
#ifdef _DEBUG
			std::cout << "LD B, (HL)" << std::endl;
#endif
			regs.B = ram[regs.HL];
			cycleWait(8);
			break;
		}
		case 0x47: // LD B, A
		{
#ifdef _DEBUG
			std::cout << "LD B, A" << std::endl;
#endif
			regs.B = regs.A;
			cycleWait(4);
			break;
		}
		case 0x4E: // LD C, (HL)
		{
#ifdef _DEBUG
			std::cout << "LD C, (HL)" << std::endl;
#endif
			regs.C = ram[regs.HL];
			cycleWait(8);
			break;
		}
		case 0x4F: // LD C, A
		{
#ifdef _DEBUG
			std::cout << "LD C, A" << std::endl;
#endif
			regs.C = regs.A;
			cycleWait(4);
			break;
		}
		case 0x54: // LD D, H
		{
#ifdef _DEBUG
			std::cout << "LD D, H" << std::endl;
#endif
			regs.D = regs.H;
			cycleWait(4);
			break;
		}
		case 0x56: // LD D, (HL)
		{
#ifdef _DEBUG
			std::cout << "LD D, (HL)" << std::endl;
#endif
			regs.D = ram[regs.HL];
			cycleWait(8);
			break;
		}
		case 0x57: // LD D, A
		{
#ifdef _DEBUG
			std::cout << "LD D, A" << std::endl;
#endif
			regs.D = regs.A;
			cycleWait(4);
			break;
		}
		case 0x5D: // LD E, L
		{
#ifdef _DEBUG
			std::cout << "LD E, L" << std::endl;
#endif
			regs.E = regs.L;
			cycleWait(4);
			break;
		}
		case 0x5F: // LD E, A
		{
#ifdef _DEBUG
			std::cout << "LD E, A" << std::endl;
#endif
			regs.E = regs.A;
			cycleWait(4);
			break;
		}
		case 0x67: // LD H, A
		{
#ifdef _DEBUG
			std::cout << "LD H, A" << std::endl;
#endif
			regs.H = regs.A;
			cycleWait(4);
			break;
		}
		case 0x6B: // LD L, E
		{
#ifdef _DEBUG
			std::cout << "LD L, E" << std::endl;
#endif
			regs.L = regs.E;
			cycleWait(4);
			break;
		}
		case 0x6E: // LD L, (HL)
		{
#ifdef _DEBUG
			std::cout << "LD L, (HL)" << std::endl;
#endif
			regs.L = ram[regs.HL];
			cycleWait(8);
			break;
		}
		case 0x6F: // LD L, A
		{
#ifdef _DEBUG
			std::cout << "LD L, A" << std::endl;
#endif
			regs.L = regs.A;
			cycleWait(4);
			break;
		}
		case 0x70: // LD (HL), B
		{
#ifdef _DEBUG
			std::cout << "LD (HL), B" << std::endl;
#endif
			ram[regs.HL] = regs.B;
			cycleWait(8);
			break;
		}
		case 0x71: // LD (HL), C
		{
#ifdef _DEBUG
			std::cout << "LD (HL), C" << std::endl;
#endif
			ram[regs.HL] = regs.C;
			cycleWait(8);
			break;
		}
		case 0x72: // LD (HL), D
		{
#ifdef _DEBUG
			std::cout << "LD (HL), D" << std::endl;
#endif
			ram[regs.HL] = regs.D;
			cycleWait(8);
			break;
		}
		case 0x77: // LD (HL), A
		{
#ifdef _DEBUG
			std::cout << "LD (HL), A" << std::endl;
#endif
			ram[regs.HL] = regs.A;
			cycleWait(8);
			break;
		}
		case 0x78: // LD A, B
		{
#ifdef _DEBUG
			std::cout << "LD A, B" << std::endl;
#endif
			regs.A = regs.B;
			cycleWait(4);
			break;
		}
		case 0x79: // LD A, C
		{
#ifdef _DEBUG
			std::cout << "LD A, C" << std::endl;
#endif
			regs.A = regs.C;
			cycleWait(4);
			break;
		}
		case 0x7A: // LD A, D
		{
#ifdef _DEBUG
			std::cout << "LD A, D" << std::endl;
#endif
			regs.A = regs.D;
			cycleWait(4);
			break;
		}
		case 0x7B: // LD A, E
		{
#ifdef _DEBUG
			std::cout << "LD A, E" << std::endl;
#endif
			regs.A = regs.E;
			cycleWait(4);
			break;
		}
		case 0x7C: // LD A, H
		{
#ifdef _DEBUG
			std::cout << "LD A, H" << std::endl;
#endif
			regs.A = regs.H;
			cycleWait(4);
			break;
		}
		case 0x7D: // LD A, L
		{
#ifdef _DEBUG
			std::cout << "LD A, L" << std::endl;
#endif
			regs.A = regs.L;
			cycleWait(4);
			break;
		}
		case 0x7E: // LD A, (HL)
		{
#ifdef _DEBUG
			std::cout << "LD A, (HL)" << std::endl;
#endif
			regs.A = ram[regs.HL];
			cycleWait(8);
			break;
		}
		case 0x83: // ADD A, E
		{
#ifdef _DEBUG
			std::cout << "ADD A, E" << std::endl;
#endif
			regs.Hf = halfcarry(regs.A, regs.E);
			regs.Cf = ((uint64_t)regs.A + regs.E) > 0xFF;
			regs.A += regs.E;
			regs.Nf = 0;
			cycleWait(4);
			break;
		}
		case 0x87: // ADD A, A
		{
#ifdef _DEBUG
			std::cout << "ADD A, A" << std::endl;
#endif
			regs.Hf = halfcarry(regs.A, regs.A);
			regs.Cf = ((uint64_t)regs.A * 2) > 0xFF;
			regs.A += regs.A;
			regs.Nf = 0;
			cycleWait(4);
			break;
		}
		case 0xA7: // AND A
		{
#ifdef _DEBUG
			std::cout << "AND A" << std::endl;
#endif
			regs.A &= regs.A;
			regs.Zf = regs.A == 0;
			regs.Nf = 0;
			regs.Hf = 1;
			regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xA9: // XOR C
		{
#ifdef _DEBUG
			std::cout << "XOR C" << std::endl;
#endif
			regs.A ^= regs.C;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xAE: // XOR (HL)
		{
#ifdef _DEBUG
			std::cout << "XOR (HL)" << std::endl;
#endif
			regs.A ^= (uint8_t)ram[regs.HL];
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(8);
			break;
		}
		case 0xAF: // XOR A
		{
#ifdef _DEBUG
			std::cout << "XOR A" << std::endl;
#endif
			regs.A ^= regs.A;
			regs.Zf = regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB0: // OR B
		{
#ifdef _DEBUG
			std::cout << "OR B" << std::endl;
#endif
			regs.A |= regs.B;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB1: // OR C
		{
#ifdef _DEBUG
			std::cout << "OR C" << std::endl;
#endif
			regs.A |= regs.C;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB2: // OR D
		{
#ifdef _DEBUG
			std::cout << "OR D" << std::endl;
#endif
			regs.A |= regs.D;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB3: // OR E
		{
#ifdef _DEBUG
			std::cout << "OR E" << std::endl;
#endif
			regs.A |= regs.E;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB4: // OR H
		{
#ifdef _DEBUG
			std::cout << "OR H" << std::endl;
#endif
			regs.A |= regs.H;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB5: // OR L
		{
#ifdef _DEBUG
			std::cout << "OR L" << std::endl;
#endif
			regs.A |= regs.L;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xB6: // OR (HL)
		{
#ifdef _DEBUG
			std::cout << "OR (HL)" << std::endl;
#endif
			regs.A |= (uint8_t)ram[regs.HL];
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(8);
			break;
		}
		case 0xB7: // OR A
		{
#ifdef _DEBUG
			std::cout << "OR A" << std::endl;
#endif
			regs.A |= regs.A;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(4);
			break;
		}
		case 0xC1: // POP BC
		{
#ifdef _DEBUG
			std::cout << "POP BC" << std::endl;
#endif
			uint16_t value = pop();
			regs.BC = value;
			cycleWait(12);
			break;
		}
		case 0xC3: // JP a16
		{
			uint16_t jumpAddress = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(jumpAddress) << " JP " << hex<uint16_t>(jumpAddress) << std::endl;
#endif
			jump(jumpAddress);
			cycleWait(16);
			break;
		}
		case 0xC4: // CALL NZ, a16
		{
			uint16_t jumpAddress = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(jumpAddress) << " CALL NZ, " << hex<uint16_t>(jumpAddress) << std::endl;
#endif
			if (!regs.Zf)
			{
				uint16_t returnAddress = regs.PC;
				push(returnAddress);
				jump(jumpAddress);
				cycleWait(24);
			}
			else
			{
				cycleWait(12);
			}
			break;
		}
		case 0xC5: // PUSH BC
		{
#ifdef _DEBUG
			std::cout << "PUSH BC" << std::endl;
#endif
			push(regs.BC);
			cycleWait(16);
			break;
		}
		case 0xC6: // ADD A, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " ADD A, " << +value << std::endl;
#endif
			regs.Hf = halfcarry(regs.A, value);
			regs.Cf = ((uint64_t)regs.A + value) > 0xFF;
			regs.A += value;
			regs.Zf = regs.A == 0;
			regs.Nf = 0;
			cycleWait(8);
			break;
		}
		case 0xC8: // RET Z
		{
#ifdef _DEBUG
			std::cout << "RET Z" << std::endl;
#endif
			if (regs.Zf)
			{
				cycleWait(20);
				uint16_t returnAddress = pop();
				jump(returnAddress);
			}
			else
			{
				cycleWait(8);
			}
			break;
		}
		case 0xC9: // RET
		{
#ifdef _DEBUG
			std::cout << "RET" << std::endl;
#endif
			uint16_t returnAddress = pop();
			jump(returnAddress);
			cycleWait(16);
			break;
		}
		case 0xCA: // JP Z, a16
		{
			uint16_t address = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(address) << " JP Z, " << hex<uint16_t>(address) << std::endl;
#endif
			if (regs.Zf)
			{
				cycleWait(16);
				jump(address);
			}
			else
			{
				cycleWait(12);
			}
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
#ifdef _DEBUG
			std::cout << hex<uint16_t>(jumpAddress) << " CALL " << hex<uint16_t>(jumpAddress) << std::endl;
#endif
			uint16_t returnAddress = regs.PC;
			push(returnAddress);
			jump(jumpAddress);
			cycleWait(24);
			break;
		}
		case 0xCE: // ADC A, d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " ADC A, " << +value << std::endl;
#endif
			uint8_t carry = regs.Cf;
			regs.Hf = halfcarry(regs.A, value + carry);
			regs.Cf = ((uint64_t)regs.A + value + carry) > 0xFF;
			regs.A += value + carry;
			regs.Zf = regs.A == 0;
			regs.Nf = 0;
			cycleWait(8);
			break;
		}
		case 0xD0: // RET NC
		{
#ifdef _DEBUG
			std::cout << "RET NC" << std::endl;
#endif
			if (!regs.Nf)
			{
				uint16_t returnAddress = pop();
				jump(returnAddress);
				cycleWait(20);
			}
			else
			{
				cycleWait(8);
			}
			break;
		}
		case 0xD1: // POP DE
		{
#ifdef _DEBUG
			std::cout << "POP DE" << std::endl;
#endif
			regs.DE = pop();
			cycleWait(12);
			break;
		}
		case 0xD5: // PUSH DE
		{
#ifdef _DEBUG
			std::cout << "PUSH DE" << std::endl;
#endif
			push(regs.DE);
			cycleWait(16);
			break;
		}
		case 0xD6: // SUB d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " SUB " << +value << std::endl;
#endif
			regs.Hf = halfcarry(regs.A, ~value + 1);
			regs.Cf = ((int64_t)regs.A - value) < 0;
			regs.A -= value;
			regs.Zf = regs.A == 0;
			regs.Nf = 1;
			cycleWait(8);
			break;
		}
		case 0xE0: // LDH (a8), A
		{
			uint8_t nextVal = nextB();
			uint16_t value = 0xFF00 + nextVal;
#ifdef _DEBUG
			std::cout << hex<uint8_t>(nextVal) << " LD (" << hex<uint16_t>(value) << "), A" << std::endl;
#endif
			ram[value] = regs.A;
			cycleWait(12);
			break;
		}
		case 0xE1: // POP HL
		{
#ifdef _DEBUG
			std::cout << "POP HL" << std::endl;
#endif
			uint16_t value = pop();
			regs.HL = value;
			cycleWait(12);
			break;
		}
		case 0xE2: // LD (C), A
		{
#ifdef _DEBUG
			std::cout << "LD (C), A" << std::endl;
#endif
			ram[regs.C] = regs.A;
			cycleWait(8);
			break;
		}
		case 0xE5: // PUSH HL
		{
#ifdef _DEBUG
			std::cout << "PUSH HL" << std::endl;
#endif
			push(regs.HL);
			cycleWait(16);
			break;
		}
		case 0xE6: // AND d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " AND " << hex<uint8_t>(value) << std::endl;
#endif
			regs.A &= value;
			regs.Zf = regs.A == 0;
			regs.Nf = 0;
			regs.Hf = 1;
			regs.Cf = 0;
			cycleWait(8);
			break;
		}
		case 0xE9: // JP (HL)
		{
#ifdef _DEBUG
			std::cout << "JP (HL)" << std::endl;
#endif
			jump(regs.HL);
			cycleWait(4);
			break;
		}
		case 0xEA: // LD (a16), A
		{
			uint16_t address = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(address) << " LD (" << hex<uint16_t>(address) << "), A" << std::endl;
#endif
			ram[address] = regs.A;
			cycleWait(16);
			break;
		}
		case 0xEE: // XOR d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " XOR " << hex<uint8_t>(value) << std::endl;
#endif
			regs.A ^= value;
			regs.Zf = regs.A == 0;
			regs.Nf = regs.Hf = regs.Cf = 0;
			cycleWait(8);
			break;
		}
		case 0xF0: // LDH A, (a8)
		{
			uint8_t nextVal = nextB();
			uint16_t value = 0xFF00 + nextVal;
#ifdef _DEBUG
			std::cout << hex<uint8_t>(nextVal) << " LD A, (" << hex<uint16_t>(value) << ")" << std::endl;
#endif
			regs.A = ram[value];
			cycleWait(12);
			break;
		}
		case 0xF1: // POP AF
		{
#ifdef _DEBUG
			std::cout << "POP AF" << std::endl;
#endif
			uint16_t value = pop();
			regs.AF = value;
			cycleWait(12);
			break;
		}
		case 0xF3: // DI Disable Interrupts
		{
#ifdef _DEBUG
			std::cout << "DI" << std::endl;
#endif
			interruptsEnabled = false;
			cycleWait(4);
			break;
		}
		case 0xF5: // PUSH AF
		{
#ifdef _DEBUG
			std::cout << "PUSH AF" << std::endl;
#endif
			push(regs.AF);
			cycleWait(16);
			break;
		}
		case 0xFA: // LD A, (a16)
		{
			uint16_t address = nextW();
#ifdef _DEBUG
			std::cout << hex<uint16_t>(address) << " LD A, (" << hex<uint16_t>(address) << ")" << std::endl;
#endif
			uint8_t value = ram[address];
			regs.A = value;
			cycleWait(16);
			break;
		}
		case 0xFB: // EI Enable Interrupts
		{
#ifdef _DEBUG
			std::cout << "EI" << std::endl;
#endif
			interruptsEnabled = true;
			cycleWait(4);
			break;
		}
		case 0xFE: // CP d8
		{
			uint8_t value = nextB();
#ifdef _DEBUG
			std::cout << hex<uint8_t>(value) << " CP " << +value << std::endl;
#endif
			regs.Zf = regs.A == value;
			regs.Nf = 1;
			regs.Hf = halfcarry(regs.A, value);
			regs.Cf = regs.A < value;
			cycleWait(8);
			break;
		}
		case 0xFF: // RST 0x38 (equivalent to CALL 0x38)
		{
#ifdef _DEBUG
			std::cout << "RST 0x38" << std::endl;
#endif
			uint16_t returnAddress = nextW();
			push(returnAddress);
			jump(0x0038);
			cycleWait(16);
			break;
		}
		default:
		{
			std::cerr << "ERR: " << hex<uint8_t>(instr) << std::endl;
			std::ostringstream sstream;
			sstream << "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << +instr << std::nouppercase;
			sstream << " at address 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << +currentPointer;
			throw OpcodeNotImplemented(sstream.str());
		}
		}
	}

	void RR(uint8_t & value)
	{
		bool carry = regs.Cf;
		regs.Cf = value & 0x01;
		value >>= 1;
		if (carry)
			value |= 0x80;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void prefixCB()
	{
		uint16_t currentPointer = regs.PC;
		uint8_t instr = nextB();
#ifdef _DEBUG
		std::cout << hex<uint8_t>(instr) << " ";
#endif
		switch (instr)
		{
		case 0x19: // RR C
		{
#ifdef _DEBUG
			std::cout << "RR C" << std::endl;
#endif
			RR(regs.C);
			cycleWait(8);
			break;
		}
		case 0x1A: // RR D
		{
#ifdef _DEBUG
			std::cout << "RR D" << std::endl;
#endif
			RR(regs.D);
			cycleWait(8);
			break;
		}
		case 0x38: // SRL B
		{
#ifdef _DEBUG
			std::cout << "SRL B" << std::endl;
#endif
			regs.Cf = regs.B & 0x01;
			regs.B >>= 1;
			regs.Zf = regs.B == 0;
			regs.Nf = regs.Hf = 0;
			cycleWait(8);
			break;
		}
		case 0x42: // BIT 0, D
		{
#ifdef _DEBUG
			std::cout << "BIT 0, D" << std::endl;
#endif
			regs.Zf = !(regs.D & 0x1);
			regs.Nf = 0;
			regs.Hf = 1;
			cycleWait(8);
			break;
		}
		case 0x87: // RES 0, A
		{
#ifdef _DEBUG
			std::cout << "RES 0, A" << std::endl;
#endif
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
		{
			step();
			display.update();
			checkInterrupts();
		}
		std::cout << "--- Executed all instructions ---" << std::endl;
		regs.dump();
	}

public:
	void OnIOWrite(uint8_t port, uint8_t value)
	{
		switch (port)
		{
		case 0x00: // Joypad
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to joypad" << std::endl;
			joypad.joypad = value;
			break;
		}
		case 0x01: // Serial Data
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to serial data" << std::endl;
			serial.data = value;
			break;
		}
		case 0x02: // Serial Tranfer Control
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to serial control" << std::endl;
			serial.control = value;
			break;
		}
		case 0x04: // Timer Divider
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer divider" << std::endl;
			timer.divider = 0x00;
			break;
		}
		case 0x05: // Timer counter
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer counter" << std::endl;
			timer.counter = value;
			break;
		}
		case 0x06: // Timer modulo
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer modulo" << std::endl;
			timer.modulo = value;
			break;
		}
		case 0x07: // Timer control
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer control" << std::endl;
			timer.control = value;
			break;
		}
		case 0x0F: // Interrupt flag
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to interrupt flag" << std::endl;
			int_flag = value;
			break;
		}
		case 0x10: // Sound - Channel 1 Sweep register
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 sweep register" << std::endl;
			sound.c1_sweep = value;
			break;
		}
		case 0x11: // Sound - Channel 1 Sound Length/Wave pattern duty
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 wave duty register" << std::endl;
			sound.c1_duty = value;
			break;
		}
		case 0x12: // Sound - Channel 1 Volume envelope
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 volume envelope register" << std::endl;
			sound.c1_envelope = value;
			break;
		}
		case 0x14: // Sound - Channel 1 Frequency High
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 frequency high register" << std::endl;
			sound.c1_frequency.upper = value;
			break;
		}
		case 0x16: // Sound - Channel 2 Sound Length/Wave pattern duty
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 wave duty register" << std::endl;
			sound.c2_duty = value;
			break;
		}
		case 0x17: // Sound - Channel 2 Volume envelope
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 volume envelope register" << std::endl;
			sound.c2_envelope = value;
			break;
		}
		case 0x19: // Sound - Channel 2 Frequency High
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 frequency high register" << std::endl;
			sound.c2_frequency.upper = value;
			break;
		}
		case 0x1A: // Sound - Channel 3 On/Off
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 on/off register" << std::endl;
			sound.c3_on = value;
			break;
		}
		case 0x1B: // Sound - Channel 3 Sound length
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 sound length register" << std::endl;
			sound.c3_sound_length = value;
			break;
		}
		case 0x1C: // Sound - Channel 3 Output level
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 output level register" << std::endl;
			sound.c3_output_level = value;
			break;
		}
		case 0x1E: // Sound - Channel 3 Frequency High
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 frequency high register" << std::endl;
			sound.c3_frequency.upper = value;
			break;
		}
		case 0x20: // Sound - Channel 3 Sound length
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 sound length register" << std::endl;
			sound.c4_sound_length = value;
			break;
		}
		case 0x21: // Sound - Channel 4 Volume envelope
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 volume envelope register" << std::endl;
			sound.c4_envelope = value;
			break;
		}
		case 0x22: // Sound - Channel 4 Polynomial counter
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 polynomial counter register" << std::endl;
			sound.c4_poly_counter = value;
			break;
		}
		case 0x23: // Sound - Channel 4 Counter consecutive
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 counter consecutive register" << std::endl;
			sound.c4_counter = value;
			break;
		}
		case 0x24: // Sound - Channel control
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to sound channel control register" << std::endl;
			sound.channel_control = value;
			break;
		}
		case 0x25: // Sound - Sound output
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to sound output register" << std::endl;
			sound.sound_output = value;
			break;
		}
		case 0x26: // Sound - Master control
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to sound master control register" << std::endl;
			sound.master_control = value;
			break;
		}
		case 0x40: // Display - LCD Control
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display control" << std::endl;
			display.control = value;
			break;
		}
		case 0x41: // Display - LCD Status
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display status" << std::endl;
			display.status = value;
			break;
		}
		case 0x42: // Display - Scroll Y
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display scroll y" << std::endl;
			display.scrollY = value;
			break;
		}
		case 0x43: // Display - Scroll X
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display scroll x" << std::endl;
			display.scrollX = value;
			break;
		}
		case 0x45: // Display - LYC
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display lyc" << std::endl;
			display.lyc = value;
			break;
		}
		case 0x47: // Display - Background palette
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display bg palette" << std::endl;
			display.bg_palette = value;
			break;
		}
		case 0x48: // Display - OBJ Palette 0
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display obj 0 palette" << std::endl;
			display.obj_palette0 = value;
			break;
		}
		case 0x49: // Display - OBJ Palette 1
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display obj 1 palette" << std::endl;
			display.obj_palette1 = value;
			break;
		}
		case 0x4A: // Display - Window Y
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display window y" << std::endl;
			display.winPosY = value;
			break;
		}
		case 0x4B: // Display - Window X
		{
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display window x" << std::endl;
			display.winPosX = value;
			break;
		}
		case 0xFF: // Interrupt master enable
		{
			interruptsEnabled = value;
			std::cout << (interruptsEnabled ? "Enabled" : "Disabled") << " interrupts" << std::endl;
			break;
		}
		default:
		{
			std::cerr << "Unimplemented IO port write " << hex<uint8_t>(port) << std::endl;
			std::ostringstream sstream;
			sstream << "Warning, writing " << hex<uint8_t>(value) << " to IO port " << hex<uint8_t>(port);
			throw IOPortNotImplemented(sstream.str());
		}
		}
	}

	uint8_t OnIORead(uint8_t port)
	{
		std::cout << "Reading from IO port " << hex<uint8_t>(port) << std::endl;
		switch (port)
		{
		case 0x00: // Keypad
			return joypad.joypad;
		case 0x01: // Serial data
			return serial.data;
		case 0x02: // Serial transfer control
			return serial.control;
		case 0x04: // Timer divider
			return timer.divider;
		case 0x05: // Timer counter
			return timer.counter;
		case 0x06: // Timer modulo
			return timer.modulo;
		case 0x07: // Timer control
			return timer.control;
		case 0x0F: // Interrupt flag
			return int_flag;
		case 0x40: // Display - LCD Control
			return display.control;
		case 0x42: // Display - Scroll Y
			return display.scrollY;
		case 0x43: // Display - Scroll X
			return display.scrollX;
		case 0x47: // Display - Background palette
			return display.bg_palette;
		case 0x48: // Display - OBJ Palette 0
			return display.obj_palette0;
		case 0x49: // Display - OBJ Palette 1
			return display.obj_palette1;
		case 0x4A: // Display - Window Y
			return display.winPosY;
		case 0x4B: // Display - Window X
			return display.winPosX;
		case 0x44: // LCD - LY
		{
			uint8_t ly = display.LY();
			std::cout << "Read LY: " << hex<uint8_t>(ly) << std::endl;
			return ly;
		}
		case 0xFF: // Interrupt master flag
			return (uint8_t)interruptsEnabled;
		default:
		{
			std::cerr << "Unimplemented IO port read " << hex<uint8_t>(port) << std::endl;
			std::ostringstream sstream;
			sstream << "Warning, reading from unimplemented IO port " << hex<uint8_t>(port);
			throw IOPortNotImplemented(sstream.str());
		}
		}
		return 0;
	}

	void interrupt(uint8_t value)
	{
		switch (value)
		{
		case 0x40: // V-Blank interrupt
			int_flag.vblank = 1;
			break;
		case 0x50:
			int_flag.timer = 1;
			break;
		default:
		{
			std::ostringstream sstream;
			sstream << "Invalid interrupt value: " << hex<uint8_t>(value);
			throw std::runtime_error(sstream.str());
		}
		}
	}

private:
	bool halfcarry(uint8_t a, uint8_t b)
	{
		return (((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10;
	}
	
	bool halfcarry(uint16_t a, uint16_t b)
	{
		return halfcarry(((uint8_t*)(&a))[1], ((uint8_t*)(&b))[1]);
	}

	void checkInterrupts()
	{
		if (!interruptsEnabled)
			return;
		if (int_enable.vblank && int_flag.vblank)
			executeInterrupt(0x40);
		else if (int_enable.timer && int_flag.timer)
			executeInterrupt(0x50);
	}

	void executeInterrupt(uint8_t value)
	{
		interruptsEnabled = false;
		push(regs.PC);
		jump(value);
	}
};

