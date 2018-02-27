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

