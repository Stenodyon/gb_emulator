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
