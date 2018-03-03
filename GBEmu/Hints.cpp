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
        if (components[0] == "l")
        {
            if (components.size() != 3)
            {
                std::cerr << "parse error: " << line << std::endl;
                continue;
            }
            uint64_t address = std::stoull(components[1], 0, 0);
            std::string name = components[2];
            hints.labels.push_back({ address, name });
        }
    }

    return hints;
}
