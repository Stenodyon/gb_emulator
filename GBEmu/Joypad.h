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

struct pad_status
{
    bool button_a, button_b, button_select, button_start;
    bool right, left, up, down;
};

class Joypad
{
private:
    struct _joypad_register
    {
        bool button_a, button_b, button_select, button_start;
        bool right, left, up, down;

        union {
            uint8_t value;
#pragma pack(push, 1)
            struct {
                uint8_t right_A : 1;
                uint8_t left_B : 1;
                uint8_t up_select : 1;
                uint8_t down_start : 1;
                uint8_t select_directions : 1;
                uint8_t select_buttons : 1;
            };
#pragma pack(pop)
        };

        _joypad_register& operator=(uint8_t value)
        {
            this->value = (value & 0xF0) | (this->value & 0x0F); return *this;
        }
        operator uint8_t()
        {
            right_A = ~((!select_directions && right) | (!select_buttons && button_a));
            left_B = ~((!select_directions && left) | (!select_buttons && button_b));
            up_select = ~((!select_directions && up) | (!select_buttons && button_select));
            down_start = ~((!select_directions && down) | (!select_buttons && button_start));
            return this->value | 0xC0; // 0 means button is pressed
        }
    };
public:
    _joypad_register joypad;

    Joypad() {}
};

