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
#include "registers.h"
#include "RAM.h"
#include "Disassembler.h"
#include "CLParser.h"

#include <fstream>

#ifndef __GNUC__
#define __attribute__(x)
#endif

void checkCGB(uint8_t * data)
{
    uint8_t cgb_flag = *(data + 0x0143);
    if ((cgb_flag & 0xC0) == 0x80)
        std::cout << "CGB mode optional" << std::endl;
    else if ((cgb_flag & 0xC0) == 0xC0)
        std::cout << "CGB mode required" << std::endl;
    else
        std::cout << "Non CGB mode" << std::endl;
}

void checkSGB(uint8_t * data)
{
    uint8_t sgb_flag = *(data + 0x0146);
    switch (sgb_flag)
    {
    case 0x00:
        std::cout << "No SGB" << std::endl;
        break;
    case 0x03:
        std::cout << "SGB supported" << std::endl;
        break;
    default:
        std::cout << "SGB disabled" << std::endl;
        break;
    }
}

void checkMBC(uint8_t * data)
{
    uint8_t mbc_type = *(data + 0x0147);
    switch (mbc_type)
    {
    case 0x00:
        std::cout << "ROM only" << std::endl;
        break;
    case 0x03:
        std::cout << "BATTERY ";
        __attribute__((fallthrough));
    case 0x02:
        std::cout << "RAM ";
        __attribute__((fallthrough));
    case 0x01:
        std::cout << "MBC1" << std::endl;
        break;
    case 0x06:
        std::cout << "BATTERY ";
        __attribute__((fallthrough));
    case 0x05:
        std::cout << "MBC2" << std::endl;
        __attribute__((fallthrough));
    case 0x09:
        std::cout << "BATTERY ";
        __attribute__((fallthrough));
    case 0x08:
        std::cout << "RAM ROM" << std::endl;
        break;
    case 0x0D:
        std::cout << "BATTERY ";
        __attribute__((fallthrough));
    case 0x0C:
        std::cout << "RAM ";
        __attribute__((fallthrough));
    case 0x0B:
        std::cout << "MMM01" << std::endl;
        break;
    case 0x0F:
        std::cout << "BATTERY TIMER MBC3" << std::endl;
        break;
    case 0x10:
        std::cout << "BATTERY RAM TIMER MBC3" << std::endl;
        break;
    case 0x11:
        std::cout << "MBC3" << std::endl;
        break;
    case 0x12:
        std::cout << "RAM MBC3" << std::endl;
        break;
    case 0x13:
        std::cout << "BATTERY RAM MBC3" << std::endl;
        break;
    case 0x1B:
        std::cout << "BATTERY ";
        __attribute__((fallthrough));
    case 0x1A:
        std::cout << "RAM ";
        __attribute__((fallthrough));
    case 0x19:
        std::cout << "MMM01" << std::endl;
        break;
    case 0x1E:
        std::cout << "BATTERY ";
        __attribute__((fallthrough));
    case 0x1D:
        std::cout << "RAM ";
        __attribute__((fallthrough));
    case 0x1C:
        std::cout << "RUMBLE MBC5" << std::endl;
        break;
    case 0x20:
        std::cout << "MBC6" << std::endl;
        break;
    case 0x22:
        std::cout << "BATTERY RAM RUMBLE SENSOR MBC7" << std::endl;
        break;
    case 0xFC:
        std::cout << "POCKET CAMERA" << std::endl;
        break;
    case 0xFD:
        std::cout << "BANDAI TAMA5" << std::endl;
        break;
    case 0xFE:
        std::cout << "HuC3" << std::endl;
        break;
    case 0xFF:
        std::cout << "BATTERY RAM HuC1" << std::endl;
        break;
    default:
        std::cout << "Unknown MBC" << std::endl;
    }
}

void romInfo(uint8_t * data)
{
    char name[17]; name[16] = 0;
    uint8_t * name_ptr = data + 0x0134;
    for (uint8_t index = 0; index < 16; index++)
    {
        name[index] = *(name_ptr + index);
        if (name[index] == 0)
            break;
    }
    std::cout << "Game name: [" << name << "]" << std::endl;
    checkCGB(data);
    checkSGB(data);
    checkMBC(data);
}

void print_usage()
{
    std::cout
        << "Usage: GBEmu [-d [-h hint_file]] rom_file" << std::endl
        << std::endl
        << "        -d              Disassemble mode" << std::endl
        << "DISASSEMBLE MODE OPTIONS:" << std::endl
        << "        -h hint_file    Provide an hint file to guide disassembly" << std::endl;
}

#ifdef _TESTING
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#else
int main(int argc, char * argv[])
{
#ifdef _DEBUG
    std::cout << "Debug build" << std::endl;
#endif
    CLParser args(argc, argv);

    std::string rom_filename = args.get_last();
    if (rom_filename == "")
    {
        print_usage();
        return -1;
    }
    uint64_t fileSize = file_size(rom_filename);
    std::cout << "ROM is " << fileSize << "B" << std::endl;
    uint8_t * data = (uint8_t*)malloc(fileSize);
    if (data == NULL)
    {
        std::cerr << "Could not allocate memory for the ROM" << std::endl;
        getchar();
        return -1;
    }
    std::cout << "Allocated memory" << std::endl;
    std::ifstream romfile;
    romfile.open(rom_filename, std::ios::in | std::ios::binary);
    romfile.read((char*)data, fileSize);
    romfile.close();
    std::cout << "Read file" << std::endl;
    romInfo(data);
    Cartridge cart(data, fileSize);

    if (args.has_option("-d"))
    {
        if (args.has_option("-h"))
        {
            std::string hints_filename = args.get_option("-h");
            if (hints_filename == "")
            {
                std::cerr << "You must specify a hint file with option -h" << std::endl;
                print_usage();
                return -1;
            }
            Hints hints = Hints::from_file(hints_filename);
            Disassembler disass(&cart, &hints);
            disass.disassemble();
            disass.dump();
        }
        else
        {
            Disassembler disass(&cart);
            disass.disassemble();
            disass.dump();
        }
    }
    else
    {
        CPU cpu = CPU(&cart);
#ifdef _DEBUG
        try {
            //cpu.breakpoints.push_back(0xB05);
            //cpu.breakpoints.push_back(0x0A03);
#endif
            cpu.breakpoints.push_back(0x100);
            cpu.run();
#ifdef _DEBUG
        }
        catch (OpcodeNotImplemented e)
        {
            std::cerr << "Exception caught:" << std::endl;
            std::cerr << "Unimplemented OpCode: " << e.what() << std::endl;
        }
        catch (std::runtime_error e)
        {
            std::cerr << "Exception caught:" << std::endl;
            std::cerr << e.what() << std::endl;
        }
#endif
    }
    free(data);
    return 0;
}
#endif

