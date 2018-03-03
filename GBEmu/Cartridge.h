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

extern const uint16_t HEADER_OFFSET;

enum MBC
{
	UNKNOWN,
	ROM,
	MBC1, MBC2, MBC3,
	MMM01,
	MBC5, MBC6, MBC7
};

class Cartridge
{
private:
	struct cart_header
	{
		uint8_t entry_point[4];
		uint8_t logo[48];
		union {
			uint8_t legacy_title[16];
			struct {
				uint8_t title[11];
				uint8_t manufacturer[4];
				uint8_t cgb_flag;
			};
		};
		uint8_t license_code[2];
		uint8_t sgb_flag;
		uint8_t mbc_flag;
		uint8_t rom_size;
		uint8_t ram_size;
		uint8_t destination_code;
		uint8_t legacy_license_code;
		uint8_t game_version;
		uint8_t checksum;
		uint8_t global_checksum[2];

		uint64_t get_ram_size() const;
		MBC get_mbc() const;
	};
	static_assert(sizeof(cart_header) == 0x50, "struct cat_header is not 80 bytes long");

public:
	uint8_t * rom;
	uint64_t rom_size;
	uint8_t * ram;
	uint64_t ram_size;
	cart_header * header;

	Cartridge(uint8_t * data, uint64_t size);
	~Cartridge();

	uint8_t read(uint8_t rom_bank, uint16_t address);
	uint8_t read_ram(uint8_t ram_bank, uint16_t address);
	void write_ram(uint8_t ram_bank, uint16_t address, uint8_t value);
};

