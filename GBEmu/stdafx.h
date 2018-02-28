// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define SDL_MAIN_HANDLED

#include "targetver.h"

#include <iostream>
#include <tchar.h>
#include <stdint.h>
#include <sstream>
#include <iomanip>
#include <cassert>

template <typename T>
struct hex
{
	T value;

	hex(T value_arg) : value(value_arg) {}
};

template <typename T>
std::ostream & operator<<(std::ostream & out, hex<T> val)
{
	bool negative = val.value < 0;
	out << (negative ? "-0x" : "0x")
		<< std::uppercase
		<< std::setfill('0')
		<< std::setw(sizeof(T) * 2)
		<< std::hex
		<< (negative ? -(val.value) : +(val.value))
		<< std::nouppercase
		<< std::dec;
	return out;
}

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
