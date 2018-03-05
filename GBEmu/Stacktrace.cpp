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
#include "Stacktrace.h"

#include <sstream>

#include "hex.h"

void Stacktrace::push(uint32_t from, uint32_t to)
{
    stack_trace.push_back({ from, to });
}

void Stacktrace::pop()
{
    if (stack_trace.size() != 0)
        stack_trace.pop_back();
    else
        std::cerr << "RET with no matching call" << std::endl;
}

void Stacktrace::dump()
{
    std::cerr << "Stack trace:" << std::endl;
    for (auto _trace : stack_trace)
    {
        std::cerr << "[" << hex<uint32_t>(_trace.first) << "] Call to"
            << hex<uint32_t>(_trace.second) << std::endl;
    }
    std::cerr << "---" << std::endl;
}