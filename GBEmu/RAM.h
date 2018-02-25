#pragma once

#include <fstream>

class CPU;

enum MBCType
{
	ROM,
	MBC1
};

class RAM
{
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
			if (address == 0xFF02 && value == 0x81)
				std::cout << (char)(uint8_t)ram[0xFF01];
			if (address >= 0x8000 && address <= 0x9FFF
				|| address >= 0xA000 && address <= 0xBFFF && ram.ramEnabled
				|| address >= 0xC000 && address <= 0xDFFF
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
			if (address >= 0xA000 && address <= 0xBFFF && !ram.ramEnabled)
				return 0;
			uint8_t * block = getMemoryBlock();
			return *(uint16_t*)(block + (address & addressMask));
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
			return MBCType::MBC1;
		default:
			std::cerr << "Unknown MBC 0x" << std::hex << mbc_flag << std::endl;
			exit(-1);
		}
	}

	void OnRamEnableWrite(uint8_t value)
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		ramEnabled = (value & 0x0F) == 0x0A;
		std::cout << "External ram " << (ramEnabled ? "enabled" : "disabled") << std::endl;
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
		std::cout << (ramMode ? "ROM" : "RAM") << " selection mode" << std::endl;
	}

	void UpdateROMPointer()
	{
#ifdef _DEBUG
		assert(type != MBCType::ROM);
#endif
		uint8_t * pointer = cartridge + ROMBank.value * 0x4000;
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

