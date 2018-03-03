#pragma once

#include "Cartridge.h"

class CPU;

class RAM
{
private:
	Cartridge * cart;
	CPU * cpu;

	const MBC mbc_type;

	bool ram_enabled = false;
	_REGISTER(rom_bank, uint8_t lower : 5; uint8_t upper : 3;)
	uint8_t ram_bank = 0;
	bool ram_mode = false;

	uint8_t work_ram[0x2000];
	uint8_t high_ram[0x7F];

public:
	RAM(Cartridge * cart, CPU * cpu);

	uint8_t read(uint16_t address);
	void writeB(uint16_t address, uint8_t value);
	void writeW(uint16_t address, uint16_t value);
};

