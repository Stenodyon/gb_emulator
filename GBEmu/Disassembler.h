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

#include "stdafx.h"

#include <list>
#include <map>

#include "Cartridge.h"
#include "Hints.h"

enum ByteType
{
    Unknown,
    Inst,
    Operand,
    Data
};

std::ostream & operator<<(std::ostream & out, ByteType & byte_type);

class Disassembler
{
public:
    class Head {
    private:
        static uint64_t id_counter;

        Disassembler * disassembler;

        void set_instr(uint64_t address, uint8_t arity);
        void jump(uif address);
        void branch(uif address);

    public:
        Head(Disassembler * disassembler, uint64_t start_byte)
            : id(id_counter++), disassembler(disassembler), current_byte(start_byte)
        {}

        uint64_t id;
        uint64_t current_byte;

        void step();
    };
private:
    Cartridge * cart;
    ByteType * byte_type;

    std::list<Head*> heads;
    std::list<Head*> new_heads;
    std::list<Head*> removed_heads;

    std::map<uint64_t, std::string> labels;
    std::string new_label(uint64_t address);
    std::string get_label(uint64_t address);
    std::string get_pointed_label(uint64_t address);

public:
    Disassembler(Cartridge * cart, Hints * hints = nullptr);
    ~Disassembler();

    void disassemble();
    void add_head(uint64_t address);
    void on_head_finished(Head * head);
    void dump();
};

