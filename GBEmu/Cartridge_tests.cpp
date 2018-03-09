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

#ifdef _TESTING

#include "catch.hpp"

#include <cstring>
#include <memory>
#include <iostream>

#include "Cartridge.h"

uint64_t count_different(Cartridge * cart, uint8_t bank_number, uint8_t value)
{
    uint64_t counter = 0;
    for (uint16_t index = 0; index < 0x4000; index++)
    {
        if (cart->read(bank_number, index) != value)
            counter++;
    }
    return counter;
}

TEST_CASE("ROM READ CORRECTLY", "[Cartridge]")
{
    std::shared_ptr<uint8_t[]> data(new uint8_t[0x1234]);
    std::memset(data.get(), 0x00, 0x150); // Zeroing the header
    std::memset(data.get() + 0x150, 0x11, 0x1234 - 0x150); // Filling 0x11 to 0x1234
    Cartridge cart(data.get(), 0x1234);

    SECTION("DATA IS READ AT ALL")
    {
        uint8_t value = cart.read(0, 0x0800);
        REQUIRE(value == 0x11);
    }

    SECTION("DATA LIMIT IS MET")
    {
        uint8_t before = cart.read(0, 0x1233);
        uint8_t after = cart.read(0, 0x1234);
        REQUIRE(before == 0x11);
        REQUIRE(after == 0xFF);
    }
}

TEST_CASE("ROM BANKS MANAGED CORRECTLY", "[Cartridge]")
{
    // Allocate 3 banks of rom
    std::shared_ptr<uint8_t[]> data(new uint8_t[4 * 0x4000]);
    if (data.get() == nullptr)
    {
        std::cerr << "Could not allocate data" << std::endl;
        exit(-1);
    }
    std::memset(data.get(), 0x00, 0x4000); // BANK 00 filled with 0x00
    std::memset(data.get() + 0x4000, 0x44, 0x4000); // BANK 01 filled with 0x44
    std::memset(data.get() + 2 * 0x4000, 0xEE, 0x4000); // BANK 02 filled with 0xEE
    std::memset(data.get() + 3 * 0x4000, 0x77, 0x4000); // BANK 03 filled with 0x77
    auto header = (Cartridge::cart_header*)(data.get() + HEADER_OFFSET);
    header->rom_size = 0x01; // 4 banks
    Cartridge cart(data.get(), 4 * 0x4000);

    SECTION("Bank 00 is 0x00 (except for the rom size byte in the header)")
    {
        uint64_t errors = count_different(&cart, 0, 0x00);
        REQUIRE(errors == 1);
    }

    SECTION("BANK 01 IS 0x44")
    {
        uint64_t errors = count_different(&cart, 1, 0x44);
        REQUIRE(errors == 0);
    }

    SECTION("BANK 02 IS 0xEE")
    {
        uint64_t errors = count_different(&cart, 2, 0xEE);
        REQUIRE(errors == 0);
    }

    SECTION("BANK 03 IS 0x77")
    {
        uint64_t errors = count_different(&cart, 3, 0x77);
        REQUIRE(errors == 0);
    }

    SECTION("BANK 1F IS MAPPED BACK TO 3")
    {
        uint64_t errors = count_different(&cart, 0x1F, 0x77);
        REQUIRE(errors == 0);
    }
}

TEST_CASE("RAM MANAGED CORRECTLY", "[Cartridge]")
{
    // Simple cartridge with 1 bank
    std::shared_ptr<uint8_t> data((uint8_t*)malloc(0x4000 * sizeof(uint8_t)));
    std::memset(data.get(), 0x00, 0x4000); // clear cartridge

    SECTION("2kB WELL DELIMITED")
    {
        data.get()[0x149] = 0x01; // 0x01 = 2kB of RAM
        Cartridge cart(data.get(), 0x4000);

        cart.write_ram(0, 0x7FF, 0x77);
        cart.write_ram(0, 0x800, 0x11);
        uint8_t low = cart.read_ram(0, 0x7FF);
        uint8_t high = cart.read_ram(0, 0x800);
        REQUIRE(low == 0x77);
        REQUIRE(high == 0xFF);
    }

    SECTION("BANKS WELL DELIMITED")
    {
        data.get()[0x149] = 0x03; // 0x03 = 4 banks of 8kB
        Cartridge cart(data.get(), 0x4000);

        cart.write_ram(0, 0x7FF, 0xFF);
        cart.write_ram(1, 0x7FF, 0x00);
        uint8_t low = cart.read_ram(0, 0x7FF);
        uint8_t high = cart.read_ram(1, 0x7FF);
        REQUIRE(low == 0xFF);
        REQUIRE(high == 0x00);
    }
}

#endif
