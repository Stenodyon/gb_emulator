#pragma once

enum MBCType
{
	ROM,
	MBC1
};

class RAM
{
public:
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

		cell_assignment& operator=(uint8_t value)
		{
			if (address >= 0x8000 && address <= 0x9FFF
				|| address >= 0xA000 && address <= 0xBFFF && ram.ramEnabled
				|| address >= 0xC000 && address <= 0xDFFF
				|| address >= 0xFF80 && address <= 0xFFFE)
			{
				uint8_t * block = getMemoryBlock();
				block[address & addressMask] = value;
			}
			if (ram.type == MBCType::ROM)
				return *this;
			if (address >= 0x0000 && address <= 0x1FFF)
				ram.OnRamEnableWrite(value);
			else if (address >= 0x2000 && address <= 0x3FFF)
				ram.OnROMBankNumber(value);
			else if (address >= 0x4000 && address <= 0x5FFF)
				ram.OnRAMBankNumber(value);
			else if (address >= 0x6000 && address <= 0x7FFF)
				ram.OnModeSelect(value);
			return *this;
		}

		operator uint8_t() const {
			uint8_t * block = getMemoryBlock();
			return block[address & addressMask];
		}

		operator uint16_t() const {
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
		ramEnabled = (value & 0x0A) == 0x0A;
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
	RAM(uint8_t * data, size_t size)
	{
		cartridge = data;
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

	~RAM() {}

	cell_assignment operator[](uint16_t address)
	{
		return cell_assignment(*this, address);
	}
};

