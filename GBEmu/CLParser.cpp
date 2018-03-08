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

#include <algorithm>

#include "stdafx.h"
#include "CLParser.h"

CLParser::CLParser(int argc, char * argv[])
{
    for (int arg = 1; arg < argc; arg++)
        tokens.push_back(std::string(argv[arg]));
}

CLParser::~CLParser()
{
}

bool CLParser::has_option(const std::string & option_name)
{
    return std::find(tokens.begin(), tokens.end(), option_name) != tokens.end();
}

std::string CLParser::get_option(const std::string & option_name)
{
    auto iter = std::find(tokens.begin(), tokens.end(), option_name);
    if (iter == tokens.end())
        return "";
    iter++;
    if (iter == tokens.end())
        return "";
    return *iter;
}

std::string CLParser::get_last()
{
    auto last_iter = tokens.rbegin();
    if (last_iter == tokens.rend())
        return "";
    return *last_iter;
}
