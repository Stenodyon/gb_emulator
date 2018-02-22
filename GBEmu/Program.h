#pragma once

#include "registers.h"

class Program
{
private:
	uint8_t * data;
	Regs & regs;

public:
	Program(uint8_t * data_arg, Regs & regs) : data(data_arg), regs(regs) {}
	~Program() {}

	uint8_t nextB()
	{
		uint8_t value = data[regs.PC];
		regs.PC++;
		return value;
	}

	uint16_t nextW()
	{
		uint16_t value = *(uint16_t*)(data + regs.PC);
		regs.PC += 2;
		return value;
	}

	void jump(uint16_t address)
	{
		regs.PC = address;
	}

	void jumpRelative(uint8_t jump)
	{
		regs.PC += jump;
	}
};

