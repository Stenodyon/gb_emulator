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

