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

