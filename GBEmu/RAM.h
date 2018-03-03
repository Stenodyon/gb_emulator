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

