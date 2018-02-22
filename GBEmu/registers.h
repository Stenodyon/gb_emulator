#pragma once

#include <utility>
#include <stdexcept>

#include "stdafx.h"

struct Regs
{
	union
	{
		uint16_t AF;
		struct
		{
			uint8_t A;
			union
			{
				uint8_t F;
				struct
				{
					uint8_t Zf : 1;
					uint8_t Nf : 1;
					uint8_t Hf : 1;
					uint8_t Cf : 1;
					uint8_t unused : 4;
				};
			};
		};
	};
	union
	{
		uint16_t BC;
		struct
		{
			uint8_t B;
			uint8_t C;
		};
	};
	union
	{
		uint16_t DE;
		struct
		{
			uint8_t D;
			uint8_t E;
		};
	};
	union
	{
		uint16_t HL;
		struct
		{
			uint8_t H;
			uint8_t L;
		};
	};
	uint16_t SP;
	uint16_t PC;
};