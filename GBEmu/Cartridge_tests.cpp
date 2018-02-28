#include "stdafx.h"

#ifdef _TESTING

#include "catch.hpp"

#include <cstring>
#include <memory>

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
	std::shared_ptr<uint8_t> data((uint8_t*)malloc(0x1234 * sizeof(uint8_t)));
	std::memset(data.get(), 0x00, 0x150);
	std::memset(data.get() + 0x150, 0xFF, 0x1234 - 0x150);
	Cartridge cart(data.get(), 0x1234);

	SECTION("DATA IS READ AT ALL")
	{
		uint8_t value = cart.read(0, 0x0800);
		REQUIRE(value == 0xFF);
	}

	SECTION("DATA LIMIT IS MET")
	{
		uint8_t before = cart.read(0, 0x1233);
		uint8_t after = cart.read(0, 0x1234);
		REQUIRE(before == 0xFF);
		REQUIRE(after == 0x00);
	}
}

TEST_CASE("ROM BANKS MANAGED CORRECTLY", "[Cartridge]")
{
	// Allocate 3 banks of rom
	std::shared_ptr<uint8_t> data((uint8_t*)malloc(3 * 0x4000 * sizeof(uint8_t)));
	std::memset(data.get(), 0x00, 0x4000); // BANK 00 filled with 0x00
	std::memset(data.get() + 0x4000, 0x44, 0x4000); // BANK 01 filled with 0x44
	std::memset(data.get() + 2 * 0x4000, 0xEE, 0x4000); // BANK 02 filled with 0xEE
	Cartridge cart(data.get(), 3 * 0x4000);

	SECTION("BANK 00 IS 0X00")
	{
		uint64_t errors = count_different(&cart, 0, 0x00);
		REQUIRE(errors == 0);
	}

	SECTION("BANK 01 IS 0X44")
	{
		uint64_t errors = count_different(&cart, 1, 0x44);
		REQUIRE(errors == 0);
	}

	SECTION("BANK 02 IS 0XEE")
	{
		uint64_t errors = count_different(&cart, 2, 0xEE);
		REQUIRE(errors == 0);
	}

	SECTION("BANK 1F IS EMPTY")
	{
		uint64_t errors = count_different(&cart, 0x1F, 0x00);
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

		cart.write_ram(0, 0x7FF, 0xFF);
		cart.write_ram(0, 0x800, 0xFF);
		uint8_t low = cart.read_ram(0, 0x7FF);
		uint8_t high = cart.read_ram(0, 0x800);
		REQUIRE(low == 0xFF);
		REQUIRE(high == 0x00);
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