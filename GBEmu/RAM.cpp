#include "stdafx.h"
#include "RAM.h"
#include "CPU.h"

RAM::cell_assignment& RAM::cell_assignment::operator=(uint8_t value)
{
	if ((address >= 0xFF00 && address <= 0xFF7F) || address == 0xFFFF)
		ram.cpu->OnIOWrite(address & 0x00FF, value);
	return assign<uint8_t>(value);
}

RAM::cell_assignment::operator uint8_t() const {
	if ((address >= 0xFF00 && address <= 0xFF7F) || address == 0xFFFF)
		return ram.cpu->OnIORead(address & 0x00FF);
	uint8_t * block = getMemoryBlock();
	return block[address & addressMask];
}
