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

class CPU;

class Timer
{
private:
	bool getTimerIncSignal();
	void incrementCounter();

public:
	CPU * cpu;
	union
	{
		uint16_t internal_counter;
#pragma pack(push, 1)
		struct {
			uint8_t divider_lower;
			uint8_t divider;
		};
#pragma pack(pop)
	};
	uint8_t counter, modulo;
	union
	{
		uint8_t control;
#pragma pack(push, 1)
		struct
		{
			uint8_t clock_select : 2;
			uint8_t start : 1;
		};
#pragma pack(pop)
	};

	Timer(CPU * cpu) : cpu(cpu) {}
	~Timer() {}

	void OnMachineCycle(uint64_t cycles);
	void resetDivider();
	void setControlRegister(uint8_t value);
};

