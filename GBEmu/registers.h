#pragma once

#include <utility>
#include <stdexcept>

#include "stdafx.h"

#include "hex.h"

struct Regs
{
	union
	{
		uint16_t AF;
		struct
		{
			union
			{
				uint8_t F;
#pragma pack(push, 1)
				struct
				{
					uint8_t unused : 4;
					uint8_t Cf : 1;
					uint8_t Hf : 1;
					uint8_t Nf : 1;
					uint8_t Zf : 1;
				};
#pragma pack(pop)
			};
			uint8_t A;
		};
	};
	union
	{
		uint16_t BC;
#pragma pack(push, 1)
		struct
		{
			uint8_t C;
			uint8_t B;
		};
#pragma pack(pop)
	};
	union
	{
		uint16_t DE;
		struct
		{
			uint8_t E;
			uint8_t D;
		};
	};
	union
	{
		uint16_t HL;
		struct
		{
			uint8_t L;
			uint8_t H;
		};
	};
	uint16_t SP;
	uint16_t PC;

	void dump()
	{
		std::cout << "A: " << hex<uint8_t>(A) << " (" << +A << ")  F: " << hex<uint8_t>(F) << " (" << +F << ")" << std::endl;
		std::cout << "B: " << hex<uint8_t>(B) << " (" << +B << ")  C: " << hex<uint8_t>(C) << " (" << +C << ")" << std::endl;
		std::cout << "D: " << hex<uint8_t>(D) << " (" << +D << ")  E: " << hex<uint8_t>(E) << " (" << +E << ")" << std::endl;
		std::cout << "H: " << hex<uint8_t>(H) << " (" << +H << ")  L: " << hex<uint8_t>(L) << " (" << +L << ")" << std::endl;
		std::cout << "PC: " << hex<uint16_t>(PC) << " (" << +PC << ")" << std::endl;
		std::cout << "SP: " << hex<uint16_t>(SP) << " (" << +SP << ")" << std::endl;
	}
};