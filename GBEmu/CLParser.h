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
#include <string>

class CLParser
{
private:
    std::vector<std::string> tokens;

public:
    CLParser(int argc, char * argv[]);
    ~CLParser();

    bool has_option(const std::string & option_name);
    std::string get_option(const std::string & option_name);
    std::string get_last();
};

