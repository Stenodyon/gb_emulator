/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "stdafx.h"
#include "CPU.h"

#ifdef _INSTR_LOG
#define _DEBUG_OUT(o) o
#else
#define _DEBUG_OUT(o)
#endif

#define _APPLY(OFFSET, f) \
	_APPLY_REG(OFFSET + 0x0, B, f) \
	_APPLY_REG(OFFSET + 0x1, C, f) \
	_APPLY_REG(OFFSET + 0x2, D, f) \
	_APPLY_REG(OFFSET + 0x3, E, f) \
	_APPLY_REG(OFFSET + 0x4, H, f) \
	_APPLY_REG(OFFSET + 0x5, L, f) \
	_APPLY_HL(OFFSET + 0x6, f) \
	_APPLY_REG(OFFSET + 0x7, A, f)

#define _APPLY_REG(VAL, REG, o) \
	case VAL: \
	{ \
		_DEBUG_OUT(std::cout << #o << " " << #REG << std::endl;) \
		cycleWait(8); \
		o(regs.REG); \
		break; \
	}

#define _APPLY_HL(VAL, o) \
	case VAL: \
	{ \
		_DEBUG_OUT(std::cout << #o << " (HL)" << std::endl;) \
		uint8_t value = ram.read(regs.HL); \
		o(value); \
		cycleWait(16); \
		ram.writeB(regs.HL, value); \
		break; \
	}

#define BIT0(x) BIT(0, x)
#define BIT1(x) BIT(1, x)
#define BIT2(x) BIT(2, x)
#define BIT3(x) BIT(3, x)
#define BIT4(x) BIT(4, x)
#define BIT5(x) BIT(5, x)
#define BIT6(x) BIT(6, x)
#define BIT7(x) BIT(7, x)

#define RES0(x) RES(0, x)
#define RES1(x) RES(1, x)
#define RES2(x) RES(2, x)
#define RES3(x) RES(3, x)
#define RES4(x) RES(4, x)
#define RES5(x) RES(5, x)
#define RES6(x) RES(6, x)
#define RES7(x) RES(7, x)

#define SET0(x) SET(0, x)
#define SET1(x) SET(1, x)
#define SET2(x) SET(2, x)
#define SET3(x) SET(3, x)
#define SET4(x) SET(4, x)
#define SET5(x) SET(5, x)
#define SET6(x) SET(6, x)
#define SET7(x) SET(7, x)

void CPU::prefixCB()
{
    uint16_t currentPointer = regs.PC;
    uint8_t instr = nextB();
#ifdef _INSTR_LOG
    std::cout << hex<uint8_t>(instr) << " ";
#endif
    switch (instr)
    {
        _APPLY(0x00, RLC)
            _APPLY(0x08, RRC)
            _APPLY(0x10, RL)
            _APPLY(0x18, RR)
            _APPLY(0x20, SLA)
            _APPLY(0x28, SRA)
            _APPLY(0x30, SWAP)
            _APPLY(0x38, SRL)
            _APPLY(0x40, BIT0)
            _APPLY(0x48, BIT1)
            _APPLY(0x50, BIT2)
            _APPLY(0x58, BIT3)
            _APPLY(0x60, BIT4)
            _APPLY(0x68, BIT5)
            _APPLY(0x70, BIT6)
            _APPLY(0x78, BIT7)
            _APPLY(0x80, RES0)
            _APPLY(0x88, RES1)
            _APPLY(0x90, RES2)
            _APPLY(0x98, RES3)
            _APPLY(0xA0, RES4)
            _APPLY(0xA8, RES5)
            _APPLY(0xB0, RES6)
            _APPLY(0xB8, RES7)
            _APPLY(0xC0, SET0)
            _APPLY(0xC8, SET1)
            _APPLY(0xD0, SET2)
            _APPLY(0xD8, SET3)
            _APPLY(0xE0, SET4)
            _APPLY(0xE8, SET5)
            _APPLY(0xF0, SET6)
            _APPLY(0xF8, SET7)
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