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
#include "Hints.h"

#include <fstream>
#include <string>

Hints::Hints()
{
}


Hints::~Hints()
{
}

Hints Hints::from_file(std::string filename)
{
    Hints hints;
    std::string contents = read_file(filename);
    std::vector<std::string> lines = split(contents, "\n");

    for (std::string & line : lines)
    {
        line = line.substr(0, line.find(";"));
        trim_spaces(line);

        std::vector<std::string> components = split(line, "\\s+");
        if (components.size() == 0)
            continue;
        if (components[0] == "l") // Label hints
        {
            if (components.size() != 3)
            {
                std::cerr << "parse error: '" << line << "'" << std::endl;
                continue;
            }
            uint64_t address = std::stoull(components[1], 0, 0);
            std::string name = components[2];
            hints.labels.push_back({ address, name });
        }
        else if (components[0] == "d") // Data range hints
        {
            if (components.size() != 3)
            {
                std::cerr << "parse error: '" << line << "'" << std::endl;
                continue;
            }
            uint64_t start = std::stoull(components[1], 0, 0);
            uint64_t end = std::stoull(components[2], 0, 0);
            hints.data_ranges.push_back({ start, end });
        }
        else
        {
            std::cerr << "parse error: '" << line << "'" << std::endl;
            continue;
        }
    }

    return hints;
}
