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

#include <fstream>

#include "Cartridge.h"
#include "hex.h"

const uint16_t HEADER_OFFSET = 0x0100;

uint32_t Cartridge::cart_header::get_rom_bank_count() const
{
    switch (rom_size)
    {
    case 0x00: // 32KB
        return 2;
    case 0x01: // 64KB
        return 4;
    case 0x02: // 128KB
        return 8;
    case 0x03: // 256KB
        return 16;
    case 0x04: // 512KB
        return 32;
    case 0x05: // 1MB
        return 64;
    case 0x06: // 2MB
        return 128;
    case 0x07: // 4MB
        return 256;
    case 0x08: // 8MB
        return 512;
    case 0x52: // 1.1MB
        return 72;
    case 0x53: // 1.2MB
        return 80;
    case 0x54: // 1.5MB
        return 96;
    default:
        std::cerr << "INVALID ROM SIZE " << hex<uint8_t>(ram_size) << std::endl;
        exit(-1);
    }
}

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

Cartridge::Cartridge(uint8_t * rom_data, uint64_t rom_size, uint8_t * ram_data, uint64_t ram_size,
    bool rom_owner, bool ram_owner)
    : rom(rom_data), rom_size(rom_size), ram(ram_data), ram_size(ram_size),
    rom_owner(rom_owner), ram_owner(ram_owner),
    header((cart_header*)(rom_data + HEADER_OFFSET))
{
    rom_bank_count = header->get_rom_bank_count();
}

Cartridge::Cartridge(uint8_t * data, uint64_t size)
    : rom(data), rom_size(size),
    ram(nullptr), ram_size(0),
    header((cart_header*)(data + HEADER_OFFSET))
{
    ram_size = header->get_ram_size();
    if (ram_size > 0)
        ram = (uint8_t*)malloc(ram_size * sizeof(uint8_t));
    ram_owner = true;

    rom_owner = false;
    rom_bank_count = header->get_rom_bank_count();
}


Cartridge::~Cartridge()
{
    if (rom_owner && rom != nullptr)
        free(rom);
    if (ram_owner && ram != nullptr)
        free(ram);
}

uint8_t Cartridge::read(uint8_t rom_bank, uint16_t address)
{
#ifdef _DEBUG
    assert(address < 0x4000);
#endif
    uint64_t offset = (uint64_t)(rom_bank % rom_bank_count) * 0x4000 + address;
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

Cartridge Cartridge::from_file(const std::string & rom_filename, const std::string & ram_filename)
{
    std::ifstream rom_file(rom_filename, std::ios::in | std::ios::binary);
    if (!rom_file.good())
    {
        std::cerr << "Could not open file '" << rom_filename << "'" << std::endl;
        exit(-1);
    }
    rom_file.seekg(0, std::ios::end);
    size_t rom_size = rom_file.tellg();
    rom_file.seekg(0, std::ios::beg);
    uint8_t * rom_data = (uint8_t*)malloc(rom_size * sizeof(uint8_t));
    if (rom_data == nullptr)
    {
        std::cerr << "Could not allocate memory for the ROM" << std::endl;
        exit(-1);
    }
    rom_file.read((char*)rom_data, rom_size);
    rom_file.close();

    std::ifstream ram_file(ram_filename, std::ios::in | std::ios::binary);
    if (!ram_file.good()) // No save file found
        return Cartridge(rom_data, rom_size);

    ram_file.seekg(0, std::ios::end);
    size_t ram_size = ram_file.tellg();
    ram_file.seekg(0, std::ios::beg);
    uint8_t * ram_data = (uint8_t*)malloc(ram_size * sizeof(uint8_t));
    if (ram_data == nullptr)
    {
        std::cerr << "Could not allocate memory for the ram" << std::endl;
        exit(-1);
    }
    ram_file.read((char*)ram_data, ram_size);
    ram_file.close();

    return Cartridge(rom_data, rom_size, ram_data, ram_size, true, true);
}
