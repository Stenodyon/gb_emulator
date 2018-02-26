#pragma once

class CPU;

class Timer
{
private:
	double dividerElapsed = 0;
	double counterElapsed = 0;

	void updateDivider();
	void updateCounter();

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
			uint8_t start : 1;
		};
#pragma pack(pop)
	};

	Timer(CPU * cpu) : cpu(cpu) {}
	~Timer() {}

	void OnMachineCycle(uint64_t cycles);
	void update();
};

