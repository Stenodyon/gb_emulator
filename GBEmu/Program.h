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

#include "registers.h"

class Program
{
private:
    uint8_t * data;
    Regs & regs;

public:
    Program(uint8_t * data_arg, Regs & regs) : data(data_arg), regs(regs) {}
    ~Program() {}

    uint8_t nextB()
    {
        uint8_t value = data[regs.PC];
        regs.PC++;
        return value;
    }

    uint16_t nextW()
    {
        uint16_t value = *(uint16_t*)(data + regs.PC);
        regs.PC += 2;
        return value;
    }

    void jump(uint16_t address)
    {
        regs.PC = address;
    }

    void jumpRelative(uint8_t jump)
    {
        regs.PC += jump;
    }
};

