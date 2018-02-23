// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

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
	out << "0x" << std::uppercase << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << +(val.value) << std::nouppercase;
	return out;
}



// TODO: reference additional headers your program requires here
