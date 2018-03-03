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

#include <utility>
#include <vector>

#include "stdafx.h"

typedef std::pair<uif, std::string> label_hint;
typedef std::pair<uif, uif> data_range;

/**
    Disassembly hints (known instruction/data bytes)
*/
class Hints
{
public:
    Hints();
    ~Hints();

    std::vector<label_hint> labels;
    std::vector<data_range> data_ranges;

    static Hints from_file(std::string filename);
};

