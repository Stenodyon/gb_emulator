
#include "stdafx.h"
#include "CPU.h"

void CPU::step()
{
	//TODO: Maybe change the large switch to a jump table
	//TODO: Move this to .cpp file
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
	case 0x04: // INC B
	{
#ifdef _DEBUG
		std::cout << "INC B" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.B, 0x01);
		regs.B++;
		regs.Zf = regs.B == 0;
		regs.Nf = 0;
		cycleWait(4);
		break;
	}
	case 0x05: // DEC B
	{
#ifdef _DEBUG
		std::cout << "DEC B" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.B, 0xFF);
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
		regs.Hf = halfcarry8(regs.C, 0x1);
		regs.C++;
		regs.Zf = regs.C == 0x00;
		regs.Nf = 1;
		cycleWait(3);
		break;
	}
	case 0x0D: // DEC C
	{
#ifdef _DEBUG
		std::cout << "DEC C" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.C, 0xFF);
		regs.C--;
		regs.Zf = regs.C == 0;
		regs.Nf = 1;
		cycleWait(4);
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
	case 0x12: // LD (DE), A
	{
#ifdef _DEBUG
		std::cout << "LD (DE), A" << std::endl;
#endif
		ram[regs.DE] = regs.A;
		cycleWait(8);
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
	case 0x14: // INC D
	{
#ifdef _DEBUG
		std::cout << "INC D" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.D, 0x01);
		regs.D++;
		regs.Zf = regs.D == 0;
		regs.Nf = 0;
		cycleWait(4);
		break;
	}
	case 0x15: // DEC D
	{
#ifdef _DEBUG
		std::cout << "DEC D" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.D, 0xFF);
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
		regs.Hf = halfcarry16(regs.HL, regs.DE);
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
	case 0x1C: // INC E
	{
#ifdef _DEBUG
		std::cout << "INC E" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.E, 0x1);
		regs.E++;
		regs.Zf = regs.E == 0x00;
		regs.Nf = 1;
		cycleWait(3);
		break;
	}
	case 0x1D: // DEC E
	{
#ifdef _DEBUG
		std::cout << "DEC E" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.E, 0xFF);
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
		regs.Hf = halfcarry8(regs.H, 0x01);
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
		regs.Hf = halfcarry8(regs.H, 0xFF);
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
		regs.Hf = halfcarry16(regs.HL, regs.HL);
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
		regs.Hf = halfcarry8(regs.L, 0x01);
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
		regs.Hf = halfcarry8(regs.L, 0xFF);
		regs.L--;
		regs.Zf = regs.L == 0;
		regs.Nf = 1;
		cycleWait(4);
		break;
	}
	case 0x2E: // LD L, d8
	{
		uint8_t value = nextB();
#ifdef _DEBUG
		std::cout << hex<uint8_t>(value) << "LD L, " << +value << std::endl;
#endif
		regs.L = value;
		cycleWait(8);
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
		regs.Hf = halfcarry8(value, 0xFF);
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
	case 0x38: // JR C, r8
	{
		//regs.dump();
		uint8_t jump = nextB();
#ifdef _DEBUG
		std::cout << hex<uint8_t>(jump) << " JR C, " << +(int8_t)jump << std::endl;
#endif
		if (regs.Cf)
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
	case 0x3C: // INC A
	{
#ifdef _DEBUG
		std::cout << "INC A" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.A, 0x1);
		regs.A++;
		regs.Zf = regs.A == 0x00;
		regs.Nf = 1;
		cycleWait(3);
		break;
	}
	case 0x3D: // DEC A
	{
#ifdef _DEBUG
		std::cout << "DEC A" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.A, 0xFF);
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
	case 0x40: // LD B, B
	{
#ifdef _DEBUG
		std::cout << "LD B, B" << std::endl;
#endif
		regs.B = regs.B;
		cycleWait(4);
		break;
	}
	case 0x41: // LD B, C
	{
#ifdef _DEBUG
		std::cout << "LD B, C" << std::endl;
#endif
		regs.B = regs.C;
		cycleWait(4);
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
	case 0x43: // LD B, E
	{
#ifdef _DEBUG
		std::cout << "LD B, E" << std::endl;
#endif
		regs.B = regs.E;
		cycleWait(4);
		break;
	}
	case 0x44: // LD B, H
	{
#ifdef _DEBUG
		std::cout << "LD B, H" << std::endl;
#endif
		regs.B = regs.H;
		cycleWait(4);
		break;
	}
	case 0x45: // LD B, L
	{
#ifdef _DEBUG
		std::cout << "LD B, L" << std::endl;
#endif
		regs.B = regs.L;
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
	case 0x48: // LD C, B
	{
#ifdef _DEBUG
		std::cout << "LD C, B" << std::endl;
#endif
		regs.C = regs.B;
		cycleWait(4);
		break;
	}
	case 0x49: // LD C, C
	{
#ifdef _DEBUG
		std::cout << "LD C, C" << std::endl;
#endif
		regs.C = regs.C;
		cycleWait(4);
		break;
	}
	case 0x4A: // LD C, D
	{
#ifdef _DEBUG
		std::cout << "LD C, D" << std::endl;
#endif
		regs.C = regs.D;
		cycleWait(4);
		break;
	}
	case 0x4B: // LD C, E
	{
#ifdef _DEBUG
		std::cout << "LD C, E" << std::endl;
#endif
		regs.C = regs.E;
		cycleWait(4);
		break;
	}
	case 0x4C: // LD C, H
	{
#ifdef _DEBUG
		std::cout << "LD C, H" << std::endl;
#endif
		regs.C = regs.H;
		cycleWait(4);
		break;
	}
	case 0x4D: // LD C, L
	{
#ifdef _DEBUG
		std::cout << "LD C, L" << std::endl;
#endif
		regs.C = regs.L;
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
	case 0x50: // LD D, B
	{
#ifdef _DEBUG
		std::cout << "LD D, B" << std::endl;
#endif
		regs.D = regs.B;
		cycleWait(4);
		break;
	}
	case 0x51: // LD D, C
	{
#ifdef _DEBUG
		std::cout << "LD D, C" << std::endl;
#endif
		regs.D = regs.C;
		cycleWait(4);
		break;
	}
	case 0x52: // LD D, D
	{
#ifdef _DEBUG
		std::cout << "LD D, D" << std::endl;
#endif
		regs.D = regs.D;
		cycleWait(4);
		break;
	}
	case 0x53: // LD D, E
	{
#ifdef _DEBUG
		std::cout << "LD D, E" << std::endl;
#endif
		regs.D = regs.E;
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
	case 0x55: // LD D, L
	{
#ifdef _DEBUG
		std::cout << "LD D, L" << std::endl;
#endif
		regs.D = regs.L;
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
	case 0x58: // LD E, B
	{
#ifdef _DEBUG
		std::cout << "LD E, B" << std::endl;
#endif
		regs.E = regs.B;
		cycleWait(4);
		break;
	}
	case 0x59: // LD E, C
	{
#ifdef _DEBUG
		std::cout << "LD E, C" << std::endl;
#endif
		regs.E = regs.C;
		cycleWait(4);
		break;
	}
	case 0x5A: // LD E, D
	{
#ifdef _DEBUG
		std::cout << "LD E, D" << std::endl;
#endif
		regs.E = regs.D;
		cycleWait(4);
		break;
	}
	case 0x5B: // LD E, E
	{
#ifdef _DEBUG
		std::cout << "LD E, E" << std::endl;
#endif
		regs.E = regs.E;
		cycleWait(4);
		break;
	}
	case 0x5C: // LD E, H
	{
#ifdef _DEBUG
		std::cout << "LD E, H" << std::endl;
#endif
		regs.E = regs.H;
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
	case 0x5E: // LD E, (HL)
	{
#ifdef _DEBUG
		std::cout << "LD E, (HL)" << std::endl;
#endif
		regs.E = ram[regs.HL];
		cycleWait(8);
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
	case 0x60: // LD H, B
	{
#ifdef _DEBUG
		std::cout << "LD H, B" << std::endl;
#endif
		regs.H = regs.B;
		cycleWait(4);
		break;
	}
	case 0x61: // LD H, C
	{
#ifdef _DEBUG
		std::cout << "LD H, C" << std::endl;
#endif
		regs.H = regs.C;
		cycleWait(4);
		break;
	}
	case 0x62: // LD H, D
	{
#ifdef _DEBUG
		std::cout << "LD H, D" << std::endl;
#endif
		regs.H = regs.D;
		cycleWait(4);
		break;
	}
	case 0x63: // LD H, E
	{
#ifdef _DEBUG
		std::cout << "LD H, E" << std::endl;
#endif
		regs.H = regs.E;
		cycleWait(4);
		break;
	}
	case 0x64: // LD H, H
	{
#ifdef _DEBUG
		std::cout << "LD H, H" << std::endl;
#endif
		regs.H = regs.H;
		cycleWait(4);
		break;
	}
	case 0x65: // LD H, L
	{
#ifdef _DEBUG
		std::cout << "LD H, L" << std::endl;
#endif
		regs.H = regs.L;
		cycleWait(4);
		break;
	}
	case 0x66: // LD H, (HL)
	{
#ifdef _DEBUG
		std::cout << "LD H, (HL)" << std::endl;
#endif
		regs.H = ram[regs.HL];
		cycleWait(8);
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
	case 0x68: // LD L, B
	{
#ifdef _DEBUG
		std::cout << "LD L, B" << std::endl;
#endif
		regs.L = regs.B;
		cycleWait(4);
		break;
	}
	case 0x69: // LD L, C
	{
#ifdef _DEBUG
		std::cout << "LD L, C" << std::endl;
#endif
		regs.L = regs.C;
		cycleWait(4);
		break;
	}
	case 0x6A: // LD L, D
	{
#ifdef _DEBUG
		std::cout << "LD L, D" << std::endl;
#endif
		regs.L = regs.D;
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
	case 0x6C: // LD L, H
	{
#ifdef _DEBUG
		std::cout << "LD L, H" << std::endl;
#endif
		regs.L = regs.H;
		cycleWait(4);
		break;
	}
	case 0x6D: // LD L, L
	{
#ifdef _DEBUG
		std::cout << "LD L, L" << std::endl;
#endif
		regs.L = regs.L;
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
	case 0x73: // LD (HL), E
	{
#ifdef _DEBUG
		std::cout << "LD (HL), E" << std::endl;
#endif
		ram[regs.HL] = regs.E;
		cycleWait(8);
		break;
	}
	case 0x74: // LD (HL), H
	{
#ifdef _DEBUG
		std::cout << "LD (HL), H" << std::endl;
#endif
		ram[regs.HL] = regs.H;
		cycleWait(8);
		break;
	}
	case 0x75: // LD (HL), L
	{
#ifdef _DEBUG
		std::cout << "LD (HL), L" << std::endl;
#endif
		ram[regs.HL] = regs.L;
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
	case 0x7F: // LD A, A
	{
#ifdef _DEBUG
		std::cout << "LD A, A" << std::endl;
#endif
		regs.A = regs.A;
		cycleWait(4);
		break;
	}
	case 0x81: // ADD A, C
	{
#ifdef _DEBUG
		std::cout << "ADD A, C" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.A, regs.C);
		regs.Cf = ((uint64_t)regs.A + regs.C) > 0xFF;
		regs.A += regs.C;
		regs.Nf = 0;
		cycleWait(4);
		break;
	}
	case 0x83: // ADD A, E
	{
#ifdef _DEBUG
		std::cout << "ADD A, E" << std::endl;
#endif
		regs.Hf = halfcarry8(regs.A, regs.E);
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
		regs.Hf = halfcarry8(regs.A, regs.A);
		regs.Cf = ((uint64_t)regs.A * 2) > 0xFF;
		regs.A += regs.A;
		regs.Nf = 0;
		cycleWait(4);
		break;
	}
	case 0x91: // SUB C
	{
#ifdef _DEBUG
		std::cout << "SUB C" << std::endl;
#endif
		regs.Hf = halfcarry_sub(regs.A, regs.C);
		regs.Cf = ((int64_t)regs.A - regs.C) < 0;
		regs.A -= regs.C;
		regs.Zf = regs.A == 0;
		regs.Nf = 1;
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
	case 0xA8: // XOR B
	{
#ifdef _DEBUG
		std::cout << "XOR B" << std::endl;
#endif
		regs.A ^= regs.B;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
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
	case 0xAA: // XOR D
	{
#ifdef _DEBUG
		std::cout << "XOR D" << std::endl;
#endif
		regs.A ^= regs.D;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
		cycleWait(4);
		break;
	}
	case 0xAB: // XOR E
	{
#ifdef _DEBUG
		std::cout << "XOR E" << std::endl;
#endif
		regs.A ^= regs.E;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
		cycleWait(4);
		break;
	}
	case 0xAC: // XOR H
	{
#ifdef _DEBUG
		std::cout << "XOR H" << std::endl;
#endif
		regs.A ^= regs.H;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
		cycleWait(4);
		break;
	}
	case 0xAD: // XOR L
	{
#ifdef _DEBUG
		std::cout << "XOR L" << std::endl;
#endif
		regs.A ^= regs.L;
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
	case 0xBB: // CP E
	{
#ifdef _DEBUG
		std::cout << "CP E" << std::endl;
#endif
		regs.Zf = regs.A == regs.E;
		regs.Nf = 1;
		regs.Hf = halfcarry_sub(regs.A, regs.E);
		regs.Cf = regs.A < regs.E;
		cycleWait(8);
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
	case 0xC2: // JP NZ, a16
	{
		uint16_t jumpAddress = nextW();
#ifdef _DEBUG
		std::cout << hex<uint16_t>(jumpAddress) << " JP NZ, " << hex<uint16_t>(jumpAddress) << std::endl;
#endif
		if (!regs.Zf)
		{
			cycleWait(16);
			jump(jumpAddress);
		}
		else
		{
			cycleWait(12);
		}
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
		regs.Hf = halfcarry8(regs.A, value);
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
		regs.Hf = ((regs.A & 0x0F) + (value & 0x0F) + regs.Cf) > 0x0F;
		value += regs.Cf;
		regs.Cf = ((uint64_t)regs.A + value) > 0xFF;
		regs.A += value;
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
		regs.Hf = halfcarry_sub(regs.A, value);
		regs.Cf = regs.A < value;
		regs.A -= value;
		regs.Zf = regs.A == 0;
		regs.Nf = 1;
		cycleWait(8);
		break;
	}
	case 0xD8: // RET C
	{
#ifdef _DEBUG
		std::cout << "RET C" << std::endl;
#endif
		if (regs.Cf)
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
	case 0xDE: // SBC A, d8
	{
		uint8_t value = nextB();
#ifdef _DEBUG
		std::cout << hex << uint8_t > (value) << " SBC A, " << +value << std::endl;
#endif
		regs.Hf = halfcarry_sub(regs.A, value);
		value += regs.Cf;
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
	case 0xF6: // OR d8
	{
		uint8_t value = nextB();
#ifdef _DEBUG
		std::cout << hex<uint8_t>(value) << " OR " << hex<uint8_t>(value) << std::endl;
#endif
		regs.A |= value;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
		cycleWait(8);
		break;
	}
	case 0xF8: // LD HL, SP+r8
	{
		int8_t value = nextB();
#ifdef _DEBUG
		std::cout << hex<uint8_t>(value) << " LD HL, SP + " << +value << std::endl;
#endif
		regs.Hf = halfcarryM(regs.SP, value);
		regs.Cf = ((int64_t)regs.SP + value) > 0xFFFF;
		regs.HL = regs.SP + value;
		regs.Zf = regs.Nf = 0;
		cycleWait(12);
		break;
	}
	case 0xF9: // LD SP, HL
	{
#ifdef _DEBUG
		std::cout << " LD SP, HL " << std::endl;
#endif
		regs.SP = regs.HL;
		cycleWait(8);
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
		regs.Hf = halfcarry_sub(regs.A, value);
		regs.Nf = 1;
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