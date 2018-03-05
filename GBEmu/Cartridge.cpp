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

#include "Cartridge.h"
#include "hex.h"

const uint16_t HEADER_OFFSET = 0x0100;

uint64_t Cartridge::cart_header::get_ram_size() const
{
    switch (ram_size)
    {
    case 0x00: // No ram
        return 0;
    case 0x01: // 2 kB
        return 0x0800;
    case 0x02: // 8 kB
        return 0x2000;
    case 0x03: // 32 kB
        return 0x8000;
    case 0x04: // 128 kB
        return 0x20000;
    case 0x05: // 64 kB
        return 0x10000;
    default:
        std::cerr << "INVALID RAM SIZE " << hex<uint8_t>(ram_size) << std::endl;
        exit(-1);
    }
}

MBC Cartridge::cart_header::get_mbc() const
{
    switch (mbc_flag)
    {
    case 0x00: case 0x08: case 0x09:
        return MBC::ROM;
    case 0x01: case 0x02: case 0x03:
        return MBC::MBC1;
    case 0x05: case 0x06:
        return MBC::MBC2;
    case 0x0B: case 0x0C: case 0x0D:
        return MBC::MMM01;
    case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13:
        return MBC::MBC3;
    case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:
        return MBC::MBC5;
    case 0x20:
        return MBC::MBC6;
    case 0x22:
        return MBC::MBC7;
    default:
        return MBC::UNKNOWN;
    }
}

Cartridge::Cartridge(uint8_t * data, uint64_t size)
    : rom(data), rom_size(size), header((cart_header*)(data + HEADER_OFFSET))
{
    ram_size = header->get_ram_size();
    if (ram_size > 0)
        ram = (uint8_t*)malloc(ram_size * sizeof(uint8_t));
}


Cartridge::~Cartridge()
{
    if (ram != nullptr)
        free(ram);
}

uint8_t Cartridge::read(uint8_t rom_bank, uint16_t address)
{
#ifdef _DEBUG
    assert(address < 0x4000);
#endif
    uint64_t offset = (uint64_t)rom_bank * 0x4000 + address;
    if (offset < rom_size)
        return rom[offset];
    return 0xFF;
}

uint8_t Cartridge::read_ram(uint8_t ram_bank, uint16_t address)
{
#ifdef _DEBUG
    assert(ram_bank < 0x10);
    assert(address < 0x2000);
#endif
    uint64_t offset = (uint64_t)ram_bank * 0x2000 + address;
    if (offset < ram_size)
        return ram[offset];
    return 0xFF;
}

void Cartridge::write_ram(uint8_t ram_bank, uint16_t address, uint8_t value)
{
#ifdef _DEBUG
    assert(ram_bank < 0x10);
    assert(address < 0x2000);
#endif
    uint64_t offset = (uint64_t)ram_bank * 0x2000 + address;
    if (offset < ram_size)
        ram[offset] = value;
}
