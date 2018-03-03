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
#include "Disassembler.h"

#include "hex.h"

#define MATH_OP(offset, name, func) \
    func(offset + 0x00, 0, name " B") \
    func(offset + 0x01, 0, name " C") \
    func(offset + 0x02, 0, name " D") \
    func(offset + 0x03, 0, name " E") \
    func(offset + 0x04, 0, name " H") \
    func(offset + 0x05, 0, name " L") \
    func(offset + 0x06, 0, name " (HL)") \
    func(offset + 0x07, 0, name " A")

#define LD_HL(offset, name, func) \
    func(offset + 0x00, 0, name " B") \
    func(offset + 0x01, 0, name " C") \
    func(offset + 0x02, 0, name " D") \
    func(offset + 0x03, 0, name " E") \
    func(offset + 0x04, 0, name " H") \
    func(offset + 0x05, 0, name " L") \
    func(offset + 0x06, 0, "HALT") \
    func(offset + 0x07, 0, name " A")

#define LDS(func) \
    MATH_OP(0x40, "LD B,", func) \
    MATH_OP(0x48, "LD C,", func) \
    MATH_OP(0x50, "LD D,", func) \
    MATH_OP(0x58, "LD E,", func) \
    MATH_OP(0x60, "LD H,", func) \
    MATH_OP(0x68, "LD L,", func) \
    LD_HL(0x70, "LD (HL),", func) \
    MATH_OP(0x78, "LD A,", func)

#define MATH_OPS(func) \
    MATH_OP(0x80, "ADD", func) \
    MATH_OP(0x88, "ADC", func) \
    MATH_OP(0x90, "SUB", func) \
    MATH_OP(0x98, "SBC", func) \
    MATH_OP(0xA0, "AND", func) \
    MATH_OP(0xA8, "XOR", func) \
    MATH_OP(0xB0, "OR", func) \
    MATH_OP(0xB8, "CP", func)

#define SWITCH_INSTR(func) \
        func(0x00, 0, "NOP") \
        func(0x10, 1, "STOP") \
\
        func(0x01, 2, "LD BC, d16") \
        func(0x11, 2, "LD DE, d16") \
        func(0x21, 2, "LD HL, d16") \
        func(0x31, 2, "LD SP, d16") \
\
        func(0x02, 0, "LD (BC), A") \
        func(0x12, 0, "LD (DE), A") \
        func(0x22, 0, "LD (HL+), A") \
        func(0x32, 0, "LD (HL-), A") \
\
        func(0x03, 0, "INC BC") \
        func(0x13, 0, "INC DE") \
        func(0x23, 0, "INC HL") \
        func(0x33, 0, "INC SP") \
\
        func(0x04, 0, "INC B") \
        func(0x14, 0, "INC D") \
        func(0x24, 0, "INC H") \
        func(0x34, 0, "INC (HL)") \
\
        func(0x05, 0, "DEC B") \
        func(0x15, 0, "DEC D") \
        func(0x25, 0, "DEC H") \
        func(0x35, 0, "DEC (HL)") \
\
        func(0x06, 1, "LD B, d8") \
        func(0x16, 1, "LD D, d8") \
        func(0x26, 1, "LD H, d8") \
        func(0x36, 1, "LD (HL), d8") \
\
        func(0x07, 0, "RLCA") \
        func(0x17, 0, "RLA") \
        func(0x27, 0, "DAA") \
        func(0x37, 0, "SCF") \
\
        func(0x08, 2, "LD (a16), SP") \
\
        func(0x09, 0, "ADD HL, BC") \
        func(0x19, 0, "ADD HL, DE") \
        func(0x29, 0, "ADD HL, HL") \
        func(0x39, 0, "ADD HL, SP") \
\
        func(0x0A, 0, "LD A, (BC)") \
        func(0x1A, 0, "LD A, (DE)") \
        func(0x2A, 0, "LD A, (HL+)") \
        func(0x3A, 0, "LD A, (HL-)") \
\
        func(0x0B, 0, "DEC BC") \
        func(0x1B, 0, "DEC DE") \
        func(0x2B, 0, "DEC HL") \
        func(0x3B, 0, "DEC SP") \
\
        func(0x0C, 0, "INC C") \
        func(0x1C, 0, "INC E") \
        func(0x2C, 0, "INC L") \
        func(0x3C, 0, "INC A") \
\
        func(0x0D, 0, "DEC C") \
        func(0x1D, 0, "DEC E") \
        func(0x2D, 0, "DEC L") \
        func(0x3D, 0, "DEC A") \
\
        func(0x0E, 1, "LD C, d8") \
        func(0x1E, 1, "LD E, d8") \
        func(0x2E, 1, "LD L, d8") \
        func(0x3E, 1, "LD A, d8") \
\
        func(0x0F, 0, "RRCA") \
        func(0x1F, 0, "RRA") \
        func(0x2F, 0, "CPL") \
        func(0x3F, 0, "CCF") \
\
        func(0xE0, 1, "LDH (a8), A") \
        func(0xF0, 1, "LDH A, (a8)") \
\
        func(0xC1, 0, "POP BC") \
        func(0xD1, 0, "POP DE") \
        func(0xE1, 0, "POP HL") \
        func(0xF1, 0, "POP AF") \
\
        func(0xE2, 0, "LDH (C), A") \
        func(0xF2, 0, "LDH A, (C)") \
\
        func(0xF3, 0, "DI") \
\
        func(0xC5, 0, "PUSH BC") \
        func(0xD5, 0, "PUSH DE") \
        func(0xE5, 0, "PUSH HL") \
        func(0xF5, 0, "PUSH AF") \
\
        func(0xC6, 1, "ADD d8") \
        func(0xD6, 1, "SUB d8") \
        func(0xE6, 1, "AND d8") \
        func(0xF6, 1, "OR d8") \
\
        func(0xE8, 1, "ADD SP, r8") \
        func(0xF8, 1, "LD HL, SP+r8") \
\
        func(0xF9, 0, "LD SP, HL") \
\
        func(0xEA, 2, "LD (a16), A") \
        func(0xFA, 2, "LD A, (a16)") \
\
        func(0xFB, 0, "EI") \
\
        func(0xCE, 1, "ADC d8") \
        func(0xDE, 1, "SBC d8") \
        func(0xEE, 1, "XOR d8") \
        func(0xFE, 1, "CP d8") \
\
        LDS(func) \
        MATH_OPS(func)

#define BIT_OP(offset, name, func) \
    MATH_OP(offset + 0x00, name " 0,", func) \
    MATH_OP(offset + 0x08, name " 1,", func) \
    MATH_OP(offset + 0x10, name " 2,", func) \
    MATH_OP(offset + 0x18, name " 3,", func) \
    MATH_OP(offset + 0x20, name " 4,", func) \
    MATH_OP(offset + 0x28, name " 5,", func) \
    MATH_OP(offset + 0x30, name " 6,", func) \
    MATH_OP(offset + 0x38, name " 7,", func)

#define SWITCH_CB(func) \
    MATH_OP(0x00, "RLC", func) \
    MATH_OP(0x08, "RRC", func) \
    MATH_OP(0x10, "RL", func) \
    MATH_OP(0x18, "RR", func) \
    MATH_OP(0x20, "SLA", func) \
    MATH_OP(0x28, "SRA", func) \
    MATH_OP(0x30, "SWAP", func) \
    MATH_OP(0x38, "SRL", func) \
    BIT_OP(0x40, "BIT", func) \
    BIT_OP(0x80, "RES", func) \
    BIT_OP(0xC0, "SET", func)

#define SWITCH_ALL(func) \
    SWITCH_INSTR(func) \
\
    func(0x20, 1, "JR NZ, r8") \
    func(0x30, 1, "JR NC, r8") \
\
    func(0x18, 1, "JR r8") \
    func(0x28, 1, "JR Z, r8") \
    func(0x38, 1, "JR C, r8") \
\
    func(0xC0, 0, "RET NZ") \
    func(0xD0, 0, "RET NC") \
\
    func(0xC2, 2, "JP NZ, a16") \
    func(0xD2, 2, "JP NC, a16") \
\
    func(0xC3, 2, "JP a16") \
\
    func(0xC4, 2, "CALL NZ, a16") \
    func(0xD4, 2, "CALL NC, a16") \
\
    func(0xC7, 0, "RST 0x00") \
    func(0xD7, 0, "RST 0x10") \
    func(0xE7, 0, "RST 0x20") \
    func(0xF7, 0, "RST 0x30") \
\
    func(0xC8, 0, "RET Z") \
    func(0xD8, 0, "RET C") \
\
    func(0xC9, 0, "RET") \
    func(0xD9, 0, "RET I") \
    func(0xE9, 0, "JP (HL)") \
\
    func(0xCA, 2, "JP Z, a16") \
    func(0xDA, 2, "JP C, a16") \
\
    func(0xCC, 2, "CALL Z, a16") \
    func(0xDC, 2, "CALL C, a16") \
\
    func(0xCD, 2, "CALL a16") \
\
    func(0xCF, 0, "RST 0x08") \
    func(0xDF, 0, "RST 0x18") \
    func(0xEF, 0, "RST 0x28") \
    func(0xFF, 0, "RST 0x38") \


#ifdef _DEBUG
#define LOG(msg) std::cout << id << " - [" << hex<uint16_t>(current_byte) << "] " << msg << std::endl;
#else
#define LOG(msg)
#endif

#define INSTR(opcode, arity, name) \
    case opcode : \
        LOG(name) \
        set_instr(arity); \
        break;

std::ostream & operator<<(std::ostream & out, ByteType & byte_type)
{
    switch (byte_type)
    {
    case ByteType::Unknown:
        return out << "Unknown";
    case ByteType::Inst:
        return out << "Instruction";
    case ByteType::Operand:
        return out << "Operand";
    case ByteType::Data:
        return out << "Data";
    default:
        return out << hex<int>(byte_type);
    }
    return out;
}

uint64_t Disassembler::Head::id_counter = 0;

void Disassembler::Head::set_instr(uint8_t arity)
{
    disassembler->byte_type[current_byte] = ByteType::Inst;
    uint16_t instr_address = current_byte;
    for (uint8_t count = 0; count < arity; count++)
    {
        current_byte++;
        ByteType byte_type = disassembler->byte_type[current_byte];
        if (byte_type != ByteType::Unknown)
        {
            std::cerr << "Byte type is not unkown at [" << hex<uint16_t>(current_byte) << "] ("
                << byte_type << ") while trying to uncover an instruction at "
                << hex<uint16_t>(instr_address) << std::endl;
            disassembler->dump();
            exit(-1);
        }
        disassembler->byte_type[current_byte] = ByteType::Operand;
    }
    current_byte++;
}

void Disassembler::Head::step()
{
    if (current_byte >= 0x4000) // Outside of bank 00
    {
        disassembler->on_head_finished(this);
        return;
    }
    if (disassembler->byte_type[current_byte] != ByteType::Unknown)
    {
        if (disassembler->byte_type[current_byte] != ByteType::Inst)
        {
            std::cerr << "Trying to read address " << hex<uint16_t>(current_byte)
                << " as an instruction but it is a(n) " << disassembler->byte_type[current_byte] << std::endl;
            disassembler->dump();
            exit(-1);
        }
        disassembler->on_head_finished(this);
        return;
    }
    uint8_t instr = disassembler->cart->rom[current_byte];

    switch (instr)
    {
        SWITCH_INSTR(INSTR)
    case 0x18: // JR r8
        LOG("JR r8");
        {
            uint16_t address = current_byte + 2 + (int8_t)(disassembler->cart->rom[current_byte + 1]);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(1);
            //disassembler->on_head_finished(this);
            current_byte = address;
        }
        break;
    case 0x20:
        LOG("JR NZ, r8")
        {
            uint16_t address = current_byte + 2 + (int8_t)(disassembler->cart->rom[current_byte + 1]);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(1);
            disassembler->add_head(address);
        }
        break;
    case 0x28: // JR Z, r8
        LOG("JR Z, r8")
        {
            uint16_t address = current_byte + 2 + (int8_t)(disassembler->cart->rom[current_byte + 1]);
            if(address < 0x4000) disassembler->get_label(address);
            disassembler->add_head(address);
            set_instr(1);
        }
        break;
    case 0x30: // JR NC, r8
        LOG("JR NC, r8")
        {
            uint16_t address = current_byte + 2 + (int8_t)(disassembler->cart->rom[current_byte + 1]);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(1);
            disassembler->add_head(address);
        }
        break;
    case 0x38: // JR C, r8
        LOG("JR C, r8")
        {
            uint16_t address = current_byte + 2 + (int8_t)(disassembler->cart->rom[current_byte + 1]);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(1);
            disassembler->add_head(address);
        }
        break;
    case 0xC0: // RET NZ
        LOG("RET NZ");
        set_instr(0);
        break;
    case 0xC2: // JP NZ, a16
        LOG("JP NZ, a16");
        {
            uint16_t address = *(uint16_t*)(disassembler->cart->rom + current_byte + 1);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(2);
            disassembler->add_head(address);
        }
        break;
    case 0xC3: // JP a16
        LOG("JP a16");
        {
            uint16_t address = *(uint16_t*)(disassembler->cart->rom + current_byte + 1);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(2);
            current_byte = address;
        }
        break;
    case 0xC8: // RET Z
        LOG("RET Z");
        set_instr(0);
        break;
    case 0xCA: // JP Z, a16
        LOG("JP Z, a16");
        {
            uint16_t address = *(uint16_t*)(disassembler->cart->rom + current_byte + 1);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(2);
            disassembler->add_head(address);
        }
        break;
    case 0xCB:
        ;
        {
            set_instr(0);
            uint8_t cb_instr = disassembler->cart->rom[current_byte];
            switch (cb_instr)
            {
                SWITCH_CB(INSTR)
            }
        }
        break;
    case 0xCD: // CALL a16
        LOG("CALL a16");
        {
            uint16_t address = *(uint16_t*)(disassembler->cart->rom + current_byte + 1);
            if(address < 0x4000) disassembler->get_label(address);
            set_instr(2);
            disassembler->add_head(address);
        }
        break;
    case 0xC9: // RET
        LOG("RET");
        set_instr(0);
        disassembler->on_head_finished(this);
        break;
    case 0xD9: // RETI
        LOG("RETI");
        set_instr(0);
        disassembler->on_head_finished(this);
        break;
    case 0xE9: // JP (HL)
        LOG("JP (HL)");
        set_instr(0);
        disassembler->on_head_finished(this);
        return;
    case 0xF7: // RST 0x30
        LOG("RST 0x30");
        set_instr(0);
        current_byte = 0x0030;
        break;
    case 0xFF: // RST 0x38
        LOG("RST 0x38");
        set_instr(0);
        current_byte = 0x0038;
        break;
    default:
        std::cerr << "Unimplemented instruction "
            << hex<uint8_t>(instr) << " at " << hex<uint16_t>(current_byte) << std::endl;
        exit(-1);
    }
}

Disassembler::Disassembler(Cartridge * cart, Hints * hints) : cart(cart) {
    byte_type = (ByteType*)malloc(cart->rom_size * sizeof(ByteType));
    if (byte_type == NULL)
    {
        std::cerr << "Unable to allocate memory for disassembly" << std::endl;
        exit(-1);
    }

    std::memset(byte_type, 0x00, cart->rom_size * sizeof(ByteType));

    for (uint16_t address = 0x104; address < 0x150; address++) // Cartridge header
        byte_type[address] = ByteType::Data;

    labels.insert({ 0x0100, "entry_point" });

    if (hints != nullptr)
    {
        for (auto label : hints->labels)
            labels.insert({ label.first, label.second });
    }
}

Disassembler::~Disassembler() {
    free(byte_type);
}

void Disassembler::disassemble()
{
    for (auto label : labels)
        heads.push_back(new Head(this, label.first));

    while (heads.size() > 0)
    {
        for (Head * head : heads)
            head->step();
        while (new_heads.size() > 0)
        {
            Head * head = new_heads.back();
            new_heads.pop_back();
            heads.push_back(head);
        }
        while (removed_heads.size() > 0)
        {
            Head * head = removed_heads.back();
            removed_heads.pop_back();
            heads.remove(head);
            delete head;
        }
    }
    std::cout << "Disassembly finished :)" << std::endl;
}

void Disassembler::add_head(uif address)
{
    Head * head = new Head(this, address);
    new_heads.push_back(head);
#ifdef _DEBUG
    std::cout << "New head spawned (" << head->id << ") at address " << hex<uint16_t>(head->current_byte) << std::endl;
#endif
}

void Disassembler::on_head_finished(Head * head)
{
#ifdef _DEBUG
    std::cout << "Head finished (" << head->id << ") at address " << hex<uint16_t>(head->current_byte) << std::endl;
#endif
    removed_heads.push_back(head);
}

std::string Disassembler::get_label(uif address)
{
    auto label_it = labels.find(address);
    if (label_it != labels.end())
        return (*label_it).second;
    // else

    std::string name = "loc_" + hex<uif>(address);
    labels.insert({address, name});
    return name;
}

#define DUMP(opcode, arity, name) \
    case opcode: \
        { \
            uint8_t arity_c = arity; \
            while(arity_c--) std::cout << " " << hex<uint8_t>(cart->rom[++address]); \
            arity_c = 2 - arity; \
            while(arity_c--) std::cout << "     "; \
            std::cout << "         " << name; \
            break; \
        }

void Disassembler::dump()
{
    auto next_label = labels.begin();
    for (uint64_t address = 0; address < 0x4000; address++)
    {
        if (next_label != labels.end() && next_label->first <= address)
        {
            std::cout << "                       " << next_label->second << ":" << std::endl;
            next_label++;
        }
        uint8_t byte = cart->rom[address];
        std::cout << "[" << hex<uint16_t>((uint16_t)address) << "] " << hex<uint8_t>(byte);
        ByteType type = byte_type[address];
        switch (type)
        {
        case ByteType::Unknown:
            std::cout << " ?";
            break;
        case ByteType::Data:
            std::cout << " data";
            break;
        case ByteType::Inst:
            switch (byte)
            {
                SWITCH_ALL(DUMP)
            case 0xCB:
                byte = cart->rom[++address];
                switch (byte)
                {
                    SWITCH_CB(DUMP)
                }
                break;
            default:
                std::cerr << " Unimplemented opcode dump " << std::endl;
                exit(-1);
            }
            break;
        case ByteType::Operand:
        default:
            std::cerr << " Unexpected byte type " << type << std::endl;
            exit(-1);
        }
        std::cout << std::endl;
    }
    if (next_label != labels.end())
        std::cerr << "Did not show all labels" << std::endl;
}