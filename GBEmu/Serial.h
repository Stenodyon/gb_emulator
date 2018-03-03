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

class Serial
{
public:
    uint8_t data;

    union
    {
        uint8_t control;
#pragma pack(push, 1)
        struct
        {
            uint8_t shift_clock : 1;
            uint8_t clock_speed : 1;
            uint8_t unused : 5;
            uint8_t start_flag : 1;
        };
#pragma pack(pop)
    };

    Serial() {}
    ~Serial() {}
};

