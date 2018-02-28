#pragma once

#include <fstream>

class CPU;

static const uint8_t BOOTSTRAP[256] = {
	0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9,
	0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
	0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06,
	0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20, 0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17,
	0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
	0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c,
	0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20, 0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50,
};


enum MBCType
{
	ROM,
	MBC1
};

class RAM
{
public:
	bool bootstrap = true;

public:
	CPU * cpu;
	uint8_t * cartridge;

	union
	{
		uint8_t memory[0x10000];
#pragma pack(push, 1)
		struct
		{
			uint8_t ROM[0x4000]; // rom bank 00
			uint8_t ROMc[0x4000]; // Switchable rom 16kB
			uint8_t VRAM[0x2000]; // Video ram
			uint8_t RAMc[0x2000]; // External ram 8kB
			uint8_t WRAM[0x2000]; // Work ram
			uint8_t SRAM[0x2000]; // Special ram
		};
#pragma pack(pop)
	};

	uint8_t * mem[8] = {
		ROM,
		ROM + 0x2000,
		ROMc,
		ROMc + 0x2000,
		VRAM,
		RAMc,
		WRAM,
		SRAM
	};

private:
	MBCType type;
	bool ramEnabled = false;
	bool ramMode = false;

	union _ROMBank
	{
		uint8_t value;
		struct
		{
			uint8_t lower5 : 5;
			uint8_t upper3 : 3;
		};
	};
	_ROMBank ROMBank;
	uint8_t RAMBank;

	/*
		Wrapper to perform operations on memory cell assignment
	*/
	class cell_assignment
	{
	private:
		static const uint16_t blockMask = 0xE000;
		static const uint16_t addressMask = ~blockMask;
		RAM & ram;
		uint16_t address;

		uint8_t * getMemoryBlock() const
		{
			uint8_t block = (address & blockMask) >> 13;
			return ram.mem[block];
		}

	public:
		cell_assignment(RAM & ram, uint16_t address) : ram(ram), address(address) {}

		template <typename T>
		cell_assignment& assign(T value)
		{
			if (ram.bootstrap && address < 0x0100)
				return *this;
			if (address == 0xFF02 && value == 0x81)
				std::cout << (char)(uint8_t)ram[0xFF01];
			if (address >= 0x8000 && address <= 0x9FFF
				|| address >= 0xA000 && address <= 0xBFFF && ram.ramEnabled
				|| address >= 0xC000 && address <= 0xDFFF
				|| address >= 0xFE00 && address <= 0xFE9F
				|| address >= 0xFF80 && address <= 0xFFFE)
			{
				uint8_t * block = getMemoryBlock();
				*(T*)(block + (address & addressMask)) = value;
			}
			if (ram.type == MBCType::ROM)
				return *this;
			if (address >= 0x0000 && address <= 0x1FFF)
				ram.OnRamEnableWrite((uint8_t)value);
			else if (address >= 0x2000 && address <= 0x3FFF)
				ram.OnROMBankNumber((uint8_t)value);
			else if (address >= 0x4000 && address <= 0x5FFF)
				ram.OnRAMBankNumber((uint8_t)value);
			else if (address >= 0x6000 && address <= 0x7FFF)
				ram.OnModeSelect((uint8_t)value);
			return *this;
		}

		cell_assignment& operator=(uint8_t value);

		cell_assignment& operator=(uint16_t value)
		{
			return assign<uint16_t>(value);
		}

		operator uint8_t() const;

		operator uint16_t() const {
			uint16_t value;
			uint8_t * ptr = (uint8_t*)&value;
			ptr[0] = ram[address];
			ptr[1] = ram[address + 1];
			return value;
#if 0
			if (address >= 0xA000 && address <= 0xBFFF && !ram.ramEnabled)
				return 0;
			uint8_t * block = getMemoryBlock();
			return *(uint16_t*)(block + (address & addressMask));
#endif
		}
	};

	MBCType getMBCType(uint8_t * data)
	{
		uint8_t mbc_flag = *(data + 0x0147);
		switch (mbc_flag)
		{
		case 0x00:
			return MBCType::ROM;
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x1B: // This will probably cause problems
			return MBCType::MBC1;
		default:
			std::cerr << "Unknown MBC " << hex<uint8_t>(mbc_flag) << std::endl;
			exit(-1);
		}
	}

	void OnRamEnableWrite(uint8_t value)
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		//ramEnabled = (value & 0x0F) == 0x0A;
		ramEnabled = value == 0x0A;
#ifdef _DEBUG
		std::cout << "External ram " << (ramEnabled ? "enabled" : "disabled") << std::endl;
#endif
	}

	void OnROMBankNumber(uint8_t value)
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		value &= 0x1F;
		if (value == 0x00)
			value = 0x01;
		ROMBank.lower5 = value;
		UpdateROMPointer();
	}

	void OnRAMBankNumber(uint8_t value)
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		value &= 0x03;
		if (ramMode)
		{
			RAMBank = value;
		}
		else
		{
			ROMBank.upper3 = value;
			UpdateROMPointer();
		}
	}

	void OnModeSelect(uint8_t value)
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		value &= 0x01;
		ramMode = value;
#ifdef _DEBUG
		std::cout << (ramMode ? "ROM" : "RAM") << " selection mode" << std::endl;
#endif
	}

	void UpdateROMPointer()
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		uint8_t * pointer = cartridge + (uint64_t)(ROMBank.value) * 0x4000;
#ifdef _DEBUG
		std::cout << "Switching to ROM BANK " << hex<uint8_t>(ROMBank.value) << std::endl;
#endif
		mem[2] = pointer;
		mem[3] = pointer + 0x2000;
	}

public:
	RAM(uint8_t * data, size_t size, CPU * cpu) : cartridge(data), cpu(cpu)
	{
		type = getMBCType(data);
		if (type == MBCType::ROM)
		{
			std::memcpy(memory, data, size);
		}
		else
		{
			std::memcpy(memory, data, 0x4000); // Bank 00
			ROMBank.value = 0x1;
			UpdateROMPointer();
		}
	}

	~RAM()
	{
	}

	cell_assignment operator[](uint16_t address)
	{
		return cell_assignment(*this, address);
	}
};

