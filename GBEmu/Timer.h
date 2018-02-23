#pragma once

class CPU;

class Timer
{
public:
	CPU * cpu;
	uint8_t divider, counter, modulo;
	union
	{
		uint8_t control;
#pragma pack(push, 1)
		struct
		{
			uint8_t clock_select : 2;
			uint8_t stop : 1;
		};
#pragma pack(pop)
	};

	Timer(CPU * cpu) : cpu(cpu) {}
	~Timer() {}

	void update();
};

