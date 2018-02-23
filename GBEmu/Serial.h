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

