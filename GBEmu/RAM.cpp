#include "stdafx.h"
#include "RAM.h"
#include "CPU.h"

RAM::RAM(Cartridge * cart, CPU * cpu)
	: cart(cart), cpu(cpu), mbc_type(cart->header->get_mbc())
{
}

uint8_t RAM::read(uint16_t address)
{
	if (address < 0x100 && bootstrap)
		return BOOTSTRAP[address];
	if (address < 0x4000) // ROM Bank 00
		return cart->read(0, address);
	if (address < 0x8000) // Switchable ROM Bank
	{
		switch (mbc_type)
		{
		case MBC::ROM:
			return cart->read(1, address - 0x4000);
		case MBC::MBC1:
			return cart->read(rom_bank, address - 0x4000);
		}
	}
	if (address < 0xA000) // Video RAM
		return cpu->display.vram[address - 0x8000];
	if (address < 0xC000) // External RAM
	{
		if (ram_enabled)
			return cart->read_ram(ram_bank, address - 0xA000);
		else
			return 0;
	}
	if (address < 0xE000) // Work RAM
		return work_ram[address - 0xC000];
	if (address < 0xFE00) // Echo RAM
		return read(0xC000 + (address - 0xE000));
	if (address < 0xFEA0) // OAM RAM
		return cpu->display.oam_ram[address - 0xFE00];
	if (address < 0xFF00) // Unusable area
		return 0;
	if (address < 0xFF80) // IO Registers
		return cpu->OnIORead(address & 0x00FF);
	if (address < 0xFFFF) // High RAM
		return high_ram[address - 0xFF80];
	return cpu->OnIORead(0xFF);
}

void RAM::writeB(uint16_t address, uint8_t value)
{
	switch (mbc_type)
	{
	case MBC::ROM:
		if (address < 0x8000) // ROM
			return;
		else if (address < 0xA000) // Video RAM
			cpu->display.vram[address - 0x8000] = value;
		else if (address < 0xC000) // External RAM
			cart->write_ram(0, address - 0xA000, value);
		else if (address < 0xE000) // Work RAM
			work_ram[address - 0xC000] = value;
		else if (address < 0xFE00) // Echo RAM
			work_ram[address - 0xC000] = value;
		else if (address < 0xFEA0) // OAM RAM
			cpu->display.oam_ram[address - 0xFE00] = value;
		else if (address < 0xFF00) // Unusable RAM
			return;
		else if (address < 0xFF80) // IO Registers
			cpu->OnIOWrite(address & 0xFF, value);
		else if (address < 0xFFFF) // High RAM
			high_ram[address - 0xFF80] = value;
		else
			cpu->OnIOWrite(0xFF, value);
		break;
	case MBC::MBC1:
		if (address < 0x2000) // RAM Enable
			ram_enabled = value == 0x0A;
		else if (address < 0x4000) // ROM bank number
		{
			uint8_t _value = value & 0x1F;
			rom_bank.lower = _value == 0 ? 0x01 : value;
			//std::cout << "New rom bank selected: " << hex<uint8_t>(rom_bank) << std::endl;
		}
		else if (address < 0x6000) // RAM/ROM bank number
		{
			if (ram_mode)
				ram_bank = value & 0x3;
			else
				rom_bank.upper = value;
		}
		else if (address < 0x8000) // Mode select
			ram_mode = value;
		else if (address < 0xA000) // Video RAM
			cpu->display.vram[address - 0x8000] = value;
		else if (address < 0xC000) // External RAM
			cart->write_ram(0, address - 0xA000, value);
		else if (address < 0xE000) // Work RAM
			work_ram[address - 0xC000] = value;
		else if (address < 0xFE00) // Echo RAM
			work_ram[address - 0xC000] = value;
		else if (address < 0xFEA0) // OAM RAM
			cpu->display.oam_ram[address - 0xFE00] = value;
		else if (address < 0xFF00) // Unusable RAM
			return;
		else if (address < 0xFF80) // IO Registers
			cpu->OnIOWrite(address & 0xFF, value);
		else if (address < 0xFFFF) // High RAM
			high_ram[address - 0xFF80] = value;
		else
			cpu->OnIOWrite(0xFF, value);
		break;
	default:
		std::cerr << "Unimplemented MBC" << std::endl;
		exit(-1);
	}
}

void RAM::writeW(uint16_t address, uint16_t value)
{
	uint8_t * ptr = (uint8_t*)&value;
	writeB(address, ptr[0]);
	writeB(address + 1, ptr[1]);
}
