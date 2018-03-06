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
#include "RAM.h"
#include "CPU.h"

RAM::RAM(Cartridge * cart, CPU * cpu)
    : cart(cart), cpu(cpu), mbc_type(cart->header->get_mbc())
{
    if(mbc_type == MBC::MBC1)
        rom_bank = 1;
}

uint8_t RAM::read(uint16_t address)
{
    if (address < 0x4000) // ROM Bank 00
        return cart->read(0, address);
    if (address < 0x8000) // Switchable ROM Bank
    {
        switch (mbc_type)
        {
        case MBC::ROM:
            return cart->read(1, address - 0x4000);
        case MBC::MBC1:
            assert(rom_bank.lower != 0);
            if (!ram_mode)
                return cart->read(rom_bank & 0x7F, address - 0x4000);
            else
                return cart->read(rom_bank & 0x1F, address - 0x4000);
        case MBC::MBC3:
            return cart->read(rom_bank & 0x7F, address - 0x4000);
        }
    }
    if (address < 0xA000) // Video RAM
        return cpu->display.vram[address - 0x8000];
    if (address < 0xC000) // External RAM
    {
        if (ram_enabled)
        {
            if (mbc_type == MBC::MBC3 && ram_bank > 0x03)
                return 0xFF; //TODO: return current RTC
            return cart->read_ram(ram_bank, address - 0xA000);
        }
        else
        {
            return 0xFF;
        }
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
            ram_enabled = (value & 0x0F) == 0x0A;
        else if (address < 0x4000) // ROM bank number
        {
            uint8_t _value = value & 0x1F;
            rom_bank.lower = _value == 0 ? 0x01 : value;
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
    case MBC::MBC3:
        if (address < 0x2000) // RAM Enable
            ram_enabled = (value & 0x0F) == 0x0A;
        else if (address < 0x4000) // ROM bank number
        {
            uint8_t _value = value & 0x7F;
            rom_bank = _value == 0 ? 0x01 : value;
        }
        else if (address < 0x6000) // RAM/RTC bank number
            rtc_register = value;
        else if (address < 0x8000) // Mode select
        {
            if (latch_clock_data == 0 && value == 0x01)
                ; //TODO: On latch clock
            latch_clock_data = value;
        }
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

uint32_t RAM::physical_address(uint16_t address)
{
    if (mbc_type == MBC::ROM)
        return address;
    if (address < 0x4000) // BANK 00
        return address;
    if (address >= 0x8000) // Not on the ROM
        return -1;

    return (uint32_t)(address - 0x4000) + 0x4000 * (uint32_t)rom_bank;
}
