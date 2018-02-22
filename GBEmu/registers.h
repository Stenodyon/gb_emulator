#pragma once

#include <utility>
#include <stdexcept>

#include "stdafx.h"

struct reg
{
	union
	{
		uint16_t value;
		struct
		{
			uint8_t low;
			uint8_t high;
		};
	};

	reg() : value(0) {}
};

enum Reg
{
	AF, A, F,
	BC, B, C,
	DE, D, E,
	HL, H, L,
	SP,
	PC
};

class InvalidRegisterException : public std::runtime_error
{
public:
	InvalidRegisterException(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class Regs
{
private:
	reg AF, BC, DE, HL, SP, PC;

public:
	uint8_t && reg8(Reg reg)
	{
		switch (reg)
		{
		case Reg::A:
			return std::move(AF.high);
		case Reg::B:
			return std::move(BC.high);
		case Reg::C:
			return std::move(BC.low);
		case Reg::D:
			return std::move(DE.high);
		case Reg::E:
			return std::move(DE.low);
		case Reg::H:
			return std::move(HL.high);
		case Reg::L:
			return std::move(HL.low);
		}
	}

	uint16_t && reg16(Reg reg)
	{
	}

	void set8(Reg reg, uint8_t value)
	{
		switch (reg)
		{
		case Reg::A:
			AF.high = value;
			break;
		case Reg::BC:
		case Reg::C:
			BC.low = value;
			break;
		case Reg::B:
			BC.high = value;
			break;
		case Reg::DE:
		case Reg::E:
			DE.low = value;
			break;
		case Reg::D:
			DE.high = value;
			break;
		case Reg::HL:
		case Reg::L:
			HL.low = value;
			break;
		case Reg::H:
			HL.high = value;
			break;
		default:
			throw InvalidRegisterException("Unexpected register");
		}
	}
};