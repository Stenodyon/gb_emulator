
#include "stdafx.h"
#include "CPU.h"

void CPU::prefixCB()
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
	case 0x1B: // RR E
	{
#ifdef _DEBUG
		std::cout << "RR E" << std::endl;
#endif
		RR(regs.E);
		cycleWait(8);
		break;
	}
	case 0x37: // SWAP A
	{
#ifdef _DEBUG
		std::cout << "SWAP A" << std::endl;
#endif
		uint8_t bottom = (regs.A & 0x0F << 4);
		regs.A >>= 4;
		regs.A |= bottom;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
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
	case 0x3F: // SRL A
	{
#ifdef _DEBUG
		std::cout << "SRL A" << std::endl;
#endif
		regs.Cf = regs.A & 0x01;
		regs.A >>= 1;
		regs.Zf = regs.A == 0;
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
		std::cerr << "ERR: 0xCB " << hex<uint8_t>(instr) << std::endl;
		std::ostringstream sstream;
		sstream << "0xCB 0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << +instr << std::nouppercase;
		sstream << " at address 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << +currentPointer;
		throw OpcodeNotImplemented(sstream.str());
	}
	}
}