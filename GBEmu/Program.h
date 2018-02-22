#pragma once

class Program
{
private:
	uint8_t * data;
	uint16_t pointer;

public:
	Program(uint8_t * data_arg) : data(data_arg), pointer(0) {}
	~Program() {}

	uint16_t getPointer() { return pointer; }

	uint8_t nextB()
	{
		uint8_t value = data[pointer];
		pointer++;
		return value;
	}

	uint16_t nextW()
	{
		uint16_t value = *(uint16_t*)(data + pointer);
		pointer += 2;
		return value;
	}

	void jump(uint16_t address)
	{
		pointer = address;
	}

	void jumpRelative(uint8_t jump)
	{
		pointer += jump;
	}
};

