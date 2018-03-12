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

#define SDL_MAIN_HANDLED

#include <iostream>
#include <tchar.h>
#include <stdint.h>
#include <sstream>
#include <iomanip>
#include <cassert>
#include <fstream>
#include <vector>

std::ifstream::pos_type file_size(const std::string& filename);
std::string read_file(std::string filename);
std::vector<std::string> split(std::string src, std::string delimiter);
void trim_spaces(std::string & str);
std::string remove_extension(std::string filename);
void write_bin(uint8_t * data, size_t size, const std::string & filename);

#define uif uint_fast16_t

#define _REGISTER(name, contents) \
struct _ ## name { \
	union { \
		uint8_t value; \
		struct { \
			contents \
		}; \
	}; \
	_ ## name & operator=(uint8_t value) { \
		this->value = value; \
		return *this; \
	}\
	operator uint8_t() const { \
		return this->value; \
	} \
}; \
static_assert(sizeof(_ ## name) == 1, #name " register is not 1 byte"); \
_ ## name name;



// TODO: reference additional headers your program requires here
