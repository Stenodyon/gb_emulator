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

        void set_instr(uint8_t arity);

    public:
        Head(Disassembler * disassembler, uif start_byte)
            : id(id_counter++), disassembler(disassembler), current_byte(start_byte)
        {}

        uint64_t id;
        uif current_byte;

        void step();
    };
private:
    Cartridge * cart;
    ByteType * byte_type;

    std::list<Head*> heads;
    std::list<Head*> new_heads;
    std::list<Head*> removed_heads;

    std::map<uif, std::string> labels;
    std::string get_label(uif address);

public:
    Disassembler(Cartridge * cart, Hints * hints = nullptr);
    ~Disassembler();

    void disassemble();
    void add_head(uif address);
    void on_head_finished(Head * head);
    void dump();
};

