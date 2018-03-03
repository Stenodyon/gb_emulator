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

#include <regex>
#include <cctype>

std::ifstream::pos_type file_size(const std::string& filename)
{
	std::streampos fsize = 0;
    std::ifstream file(filename, std::ios::binary);

	if (!file)
	{
		std::cerr << "File '" << filename << "' does not exist" << std::endl;
		exit(-1);
	}

    fsize = file.tellg();
    file.seekg(0, std::ios::end);
    fsize = file.tellg() - fsize;
    file.close();

    return fsize;
}

std::string read_file(std::string filename)
{
    std::ifstream stream(filename);
    std::string contents;

    stream.seekg(0, std::ios::end);
    contents.reserve(stream.tellg());
    stream.seekg(0, std::ios::beg);

    contents.assign((std::istreambuf_iterator<char>(stream)),
        std::istreambuf_iterator<char>());
    return contents;
}

std::vector<std::string> split(std::string src, std::string delimiter)
{
    std::regex re{ delimiter };
    std::vector<std::string> container{
        std::sregex_token_iterator(src.begin(), src.end(), re, -1),
        std::sregex_token_iterator() };
    return container;
}

void trim_spaces(std::string & str)
{
    str.erase(str.begin(),
        std::find_if(str.begin(), str.end(),
            [](char ch) { return !std::isspace(ch); }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch) { return !std::isspace(ch); }).base(),
        str.end());
}
