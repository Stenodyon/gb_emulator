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

#include <vector>
#include <cstdint>
#include <utility>

typedef std::pair<uint32_t, uint32_t> trace;

class Stacktrace
{
private:
    std::vector<trace> stack_trace;

public:
    void push(uint32_t from, uint32_t to);
    void pop(uint32_t from, uint32_t to);
    void dump();
};

