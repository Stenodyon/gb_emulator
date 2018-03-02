#pragma once

#include <stdexcept>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>

#include "stdafx.h"
#include "registers.h"
#include "RAM.h"
#include "Display.h"
#include "Serial.h"
#include "Timer.h"
#include "Sound.h"
#include "Joypad.h"

class OpcodeNotImplemented : public std::runtime_error
{
public:
	OpcodeNotImplemented(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class IOPortNotImplemented : public std::runtime_error
{
public:
	IOPortNotImplemented(const std::string& what_arg) : std::runtime_error(what_arg) {}
};

class CPU
{
public:
	std::vector<uint16_t> breakpoints;

	uint64_t cycleCount = 0;

	bool running = true;
	Joypad joypad;

	uint8_t unused_regsiter_03;
	uint8_t unused_regsiter_08;

	Regs regs;
	RAM ram;
	Display display;
	Serial serial;
	Timer timer;
	Sound sound;

private:
	bool interruptsEnabled = true;
	bool halted = false;
	bool double_speed = false;

	struct _int_flag
	{
		union
		{
			uint8_t value;
#pragma pack(push, 1)
			struct
			{
				uint8_t vblank : 1;
				uint8_t lcd_stat : 1;
				uint8_t timer : 1;
				uint8_t serial : 1;
				uint8_t joypad : 1;
			};
#pragma pack(pop)
		};

		_int_flag& operator=(uint8_t value) { this->value = value; return *this; }
		operator uint8_t() const { return this->value; }
	};
	_int_flag int_enable, int_flag;

	struct _speed_switch
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t prepare : 1;
				uint8_t unused : 6;
				uint8_t current_speed : 1;
			};
#pragma pack(pop)
		};

		_speed_switch& operator=(uint8_t value) { this->value = value & 0x01; return *this; }
		operator uint8_t() const { return this->value & 0x81; }
	};
	_speed_switch speed_switch;

public:
	CPU(Cartridge * cart) : ram(cart, this) , display(this), timer(this)
	{
		sound.init();
		jump(0x0000);
#if 0
		jump(0x100); // Entry point is 0x100

		regs.AF = 0x01B0;
		regs.BC = 0x0013;
		regs.DE = 0x00D8;
		regs.HL = 0x014D;
		regs.SP = 0xFFFE;

		ram[0xFF05] = (uint8_t)0x00;
		ram[0xFF06] = (uint8_t)0x00;
		ram[0xFF07] = (uint8_t)0x00;
		ram[0xFF10] = (uint8_t)0x80;
		ram[0xFF11] = (uint8_t)0xBF;
		ram[0xFF12] = (uint8_t)0xF3;
		ram[0xFF14] = (uint8_t)0xBF;
		ram[0xFF16] = (uint8_t)0x3F;
		ram[0xFF17] = (uint8_t)0x00;
		ram[0xFF19] = (uint8_t)0xBF;
		ram[0xFF1A] = (uint8_t)0x7F;
		ram[0xFF1B] = (uint8_t)0xFF;
		ram[0xFF1C] = (uint8_t)0x9F;
		ram[0xFF1E] = (uint8_t)0xBF;
		ram[0xFF20] = (uint8_t)0xFF;
		ram[0xFF21] = (uint8_t)0x00;
		ram[0xFF22] = (uint8_t)0x00;
		ram[0xFF23] = (uint8_t)0xBF;
		ram[0xFF24] = (uint8_t)0x77;
		ram[0xFF25] = (uint8_t)0xF3;
		ram[0xFF26] = (uint8_t)0xF1;
		ram[0xFF40] = (uint8_t)0x91;
		ram[0xFF42] = (uint8_t)0x00;
		ram[0xFF43] = (uint8_t)0x00;
		ram[0xFF45] = (uint8_t)0x00;
		ram[0xFF47] = (uint8_t)0xFC;
		ram[0xFF48] = (uint8_t)0xFF;
		ram[0xFF49] = (uint8_t)0xFF;
		ram[0xFF4A] = (uint8_t)0x00;
		ram[0xFF4B] = (uint8_t)0x00;
		ram[0xFFFF] = (uint8_t)0x00;
#endif

		std::cout << "CPU initialized" << std::endl;
	}

	~CPU() {}

	void cycleWait(uint64_t cycleCount)
	{
		assert(cycleCount % 4 == 0);
        for (uint64_t cycle = 0; cycle < cycleCount / 4; cycle++)
        {
            timer.OnMachineCycle(1);
            display.OnMachineCycle(1);
            sound.OnMachineCycle(1);
        }
		this->cycleCount += cycleCount;
	}

	uint8_t nextB()
	{
		uint8_t value = ram.read(regs.PC);
		regs.PC++;
		return value;
	}

	uint16_t nextW()
	{
		uint16_t value;
		uint8_t * ptr = (uint8_t*)&value;
		ptr[0] = ram.read(regs.PC);
		ptr[1] = ram.read(regs.PC + 1);
		regs.PC += 2;
		return value;
	}

	void jump(uint16_t address)
	{
#ifdef _DEBUG
		std::cout << "Jump to " << hex<uint16_t>(address) << std::endl;
#endif
		regs.PC = address;
	}

	void jumpRelative(uint8_t jump)
	{
		uint16_t address = regs.PC + (int8_t)jump;
		regs.PC = address;
#ifdef _DEBUG
		std::cout << "Relative jump to " << hex<uint16_t>(address) << std::endl;
#endif
	}

	void push(uint16_t value)
	{
		regs.SP -= 2;
		uint8_t * ptr = (uint8_t*)&value;
		ram.writeB(regs.SP, ptr[0]);
		cycleWait(4);
		ram.writeB(regs.SP + 1, ptr[1]);
		cycleWait(4);
	}

	uint16_t pop()
	{
		uint16_t value;
		uint8_t * ptr = (uint8_t*)&value;
		ptr[0] = ram.read(regs.SP);
		cycleWait(4);
		ptr[1] = ram.read(regs.SP + 1);
		cycleWait(4);
		regs.SP += 2;
		return value;
	}

	void step();
	void prefixCB();
	void run();
	void updateInput();

	void RLC(uint8_t & value)
	{
		regs.Cf = (value & 0x80) > 0;
		value <<= 1;
		value |= regs.Cf;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void RRC(uint8_t & value)
	{
		regs.Cf = value & 0x01;
		value >>= 1;
		value |= regs.Cf ? 0x80 : 0x00;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void RL(uint8_t & value)
	{
		uint8_t temp = regs.Cf;
		regs.Cf = (value & 0x80) > 0;
		value <<= 1;
		value |= temp;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void RR(uint8_t & value)
	{
		bool carry = regs.Cf;
		regs.Cf = value & 0x01;
		value >>= 1;
		if (carry)
			value |= 0x80;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void SLA(uint8_t & value)
	{
		regs.Cf = (value & 0x80) > 0;
		value <<= 1;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void SRA(uint8_t & value)
	{
		regs.Cf = value & 0x01;
		uint8_t bit7 = value & 0x80;
		value >>= 1;
		value |= bit7;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void SWAP(uint8_t & value)
	{
		uint8_t bottom = ((value & 0x0F) << 4);
		value >>= 4;
		value |= bottom;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
	}

	void SRL(uint8_t & value)
	{
		regs.Cf = value & 0x01;
		value >>= 1;
		regs.Zf = value == 0;
		regs.Nf = regs.Hf = 0;
	}

	void BIT(uint8_t bit, uint8_t value)
	{
#ifdef _DEBUG
		std::cout << "  -> Value = " << hex<uint8_t>(value);
#endif
		regs.Zf = !(value & (0x1 << bit));
		regs.Nf = 0;
		regs.Hf = 1;
#ifdef _DEBUG
		std::cout << ", Result = " << +(regs.Zf) << std::endl;
#endif
	}

	void RES(uint8_t bit, uint8_t & value)
	{
		value &= ~(0x1 << bit);
	}

	void SET(uint8_t bit, uint8_t & value)
	{
		value |= 0x1 << bit;
	}

	inline void ADD(uint8_t value)
	{
#ifdef _DEBUG
		std::cout << "  -> reg = " << +value << ", A = " << +(regs.A);
#endif
		regs.Hf = (regs.A & 0x0F) + (value & 0x0F) > 0x0F;
		regs.Cf = ((uint64_t)regs.A + value) > 0xFF;
		regs.A += value;
		regs.Zf = regs.A == 0;
		regs.Nf = 0;
#ifdef _DEBUG
		std::cout << ", result = " << +(regs.A) << std::endl;
#endif
	}

	inline void ADC(uint8_t value)
	{
		uint8_t carry = regs.Cf;
		regs.Hf = ((regs.A & 0x0F) + (value & 0x0F) + carry) > 0x0F;
		regs.Cf = ((uint64_t)regs.A + (uint64_t)value + carry) > 0xFF;
		regs.A += value + carry;
		regs.Zf = regs.A == 0;
		regs.Nf = 0;
	}

	inline void SUB(uint8_t value)
	{
#ifdef _DEBUG
		std::cout << "  -> reg = " << +value << ", A = " << +(regs.A);
#endif
		regs.Hf = halfcarry_sub(regs.A, value);
		regs.Cf = regs.A < value;
		regs.A -= value;
		regs.Zf = regs.A == 0;
		regs.Nf = 1;
#ifdef _DEBUG
		std::cout << ", result = " << +(regs.A) << std::endl;
#endif
	}

	inline void SBC(uint8_t value)
	{
		uint8_t carry = regs.Cf;
		regs.Hf = (uint64_t)(regs.A & 0xF) < (((uint64_t)value & 0xF) + carry);
		regs.Cf = ((int64_t)regs.A - (int64_t)value - carry) < 0;
		regs.A -= value;
		regs.A -= carry;
		regs.Zf = regs.A == 0;
		regs.Nf = 1;
	}

	inline void AND(uint8_t value)
	{
#ifdef _DEBUG
		std::cout << "  -> reg = " << hex<uint8_t>(value) << ", A = " << hex<uint8_t>(regs.A);
#endif
		regs.A &= value;
		regs.Zf = regs.A == 0;
		regs.Nf = 0;
		regs.Hf = 1;
		regs.Cf = 0;
#ifdef _DEBUG
		std::cout << ", result = " << hex<uint8_t>(regs.A) << std::endl;
#endif
	}

	inline void XOR(uint8_t value)
	{
#ifdef _DEBUG
		std::cout << "  -> reg = " << hex<uint8_t>(value) << ", A = " << hex<uint8_t>(regs.A);
#endif
		regs.A ^= value;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
#ifdef _DEBUG
		std::cout << ", result = " << hex<uint8_t>(regs.A) << std::endl;
#endif
	}

	inline void OR(uint8_t value)
	{
#ifdef _DEBUG
		std::cout << "  -> reg = " << hex<uint8_t>(value) << ", A = " << hex<uint8_t>(regs.A);
#endif
		regs.A |= value;
		regs.Zf = regs.A == 0;
		regs.Nf = regs.Hf = regs.Cf = 0;
#ifdef _DEBUG
		std::cout << ", result = " << hex<uint8_t>(regs.A) << std::endl;
#endif
	}

	inline void CP(uint8_t value)
	{
		regs.Zf = regs.A == value;
		regs.Hf = halfcarry_sub(regs.A, value);
		regs.Nf = 1;
		regs.Cf = regs.A < value;
	}

	void DMATranfer(uint8_t source);

public:
	void OnIOWrite(uint8_t port, uint8_t value)
	{
		if ((port & 0xF0) == 0x30) // Sound - Wave pattern RAM
		{
			sound.wave_pattern[port & 0x0F] = value;
			return;
		}
		switch (port)
		{
		case 0x00: // Joypad
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to joypad" << std::endl;
#endif
			joypad.joypad = value;
			break;
		}
		case 0x01: // Serial Data
		{
			//std::cout << "Wrote " << hex<uint8_t>(value) << " to serial data" << std::endl;
			serial.data = value;
			break;
		}
		case 0x02: // Serial Tranfer Control
		{
			//std::cout << "Wrote " << hex<uint8_t>(value) << " to serial control" << std::endl;
			serial.control = value;
			break;
		}
		case 0x04: // Timer Divider
		{
#ifdef _DEBUG
			std::cout << "Reset timer divider" << std::endl;
#endif
			timer.resetDivider();
			break;
		}
		case 0x05: // Timer counter
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer counter" << std::endl;
#endif
			timer.counter = value;
			break;
		}
		case 0x06: // Timer modulo
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer modulo" << std::endl;
#endif
			timer.modulo = value;
			break;
		}
		case 0x07: // Timer control
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to timer control" << std::endl;
#endif
			timer.setControlRegister(value);
			break;
		}
		case 0x0F: // Interrupt flag
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to interrupt flag" << std::endl;
#endif
			int_flag = value;
			break;
		}
		case 0x10: // Sound - Channel 1 Sweep register
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 sweep register" << std::endl;
#endif
			sound.c1_sweep = value;
			break;
		}
		case 0x11: // Sound - Channel 1 Sound Length/Wave pattern duty
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 wave duty register" << std::endl;
#endif
			sound.c1_duty = value;
			break;
		}
		case 0x12: // Sound - Channel 1 Volume envelope
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 volume envelope register" << std::endl;
#endif
			sound.c1_envelope = value;
			break;
		}
		case 0x13: // Sound - Channel 1 Frequency Low
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 frequency low register" << std::endl;
#endif
			sound.c1_frequency.lower = value;
			break;
		}
		case 0x14: // Sound - Channel 1 Frequency High
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 1 frequency high register" << std::endl;
#endif
			sound.c1_frequency.upper = value;
			break;
		}
		case 0x16: // Sound - Channel 2 Sound Length/Wave pattern duty
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 wave duty register" << std::endl;
#endif
			sound.c2_duty = value;
			break;
		}
		case 0x17: // Sound - Channel 2 Volume envelope
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 volume envelope register" << std::endl;
#endif
			sound.c2_envelope = value;
			break;
		}
		case 0x18: // Sound - Channel 2 Frequency Low
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 frequency low register" << std::endl;
#endif
			sound.c2_frequency.lower = value;
			break;
		}
		case 0x19: // Sound - Channel 2 Frequency High
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 2 frequency high register" << std::endl;
#endif
			sound.c2_frequency.upper = value;
			break;
		}
		case 0x1A: // Sound - Channel 3 On/Off
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 on/off register" << std::endl;
#endif
			sound.c3_on = value;
			break;
		}
		case 0x1B: // Sound - Channel 3 Sound length
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 sound length register" << std::endl;
#endif
			sound.c3_sound_length = value;
			break;
		}
		case 0x1C: // Sound - Channel 3 Output level
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 output level register" << std::endl;
#endif
			sound.c3_output_level = value;
			break;
		}
		case 0x1D: // Sound - Channel 3 Frequency Low
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 frequency low register" << std::endl;
#endif
			sound.c3_frequency.lower = value;
			break;
		}
		case 0x1E: // Sound - Channel 3 Frequency High
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 3 frequency high register" << std::endl;
#endif
			sound.c3_frequency.upper = value;
			break;
		}
		case 0x20: // Sound - Channel 3 Sound length
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 sound length register" << std::endl;
#endif
			sound.c4_sound_length = value;
			break;
		}
		case 0x21: // Sound - Channel 4 Volume envelope
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 volume envelope register" << std::endl;
#endif
			sound.c4_envelope = value;
			break;
		}
		case 0x22: // Sound - Channel 4 Polynomial counter
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 polynomial counter register" << std::endl;
#endif
			sound.c4_poly_counter = value;
			break;
		}
		case 0x23: // Sound - Channel 4 Counter consecutive
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to channel 4 counter consecutive register" << std::endl;
#endif
			sound.c4_counter = value;
			break;
		}
		case 0x24: // Sound - Channel control
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to sound channel control register" << std::endl;
#endif
			sound.channel_control = value;
			break;
		}
		case 0x25: // Sound - Sound output
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to sound output register" << std::endl;
#endif
			sound.sound_output = value;
			break;
		}
		case 0x26: // Sound - Master control
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to sound master control register" << std::endl;
#endif
			sound.master_control = value;
			break;
		}
		case 0x40: // Display - LCD Control
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display control" << std::endl;
#endif
			display.control = value;
			break;
		}
		case 0x41: // Display - LCD Status
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display status" << std::endl;
#endif
			display.status = value;
			break;
		}
		case 0x42: // Display - Scroll Y
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display scroll y" << std::endl;
#endif
			display.scrollY = value;
			break;
		}
		case 0x43: // Display - Scroll X
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display scroll x" << std::endl;
#endif
			display.scrollX = value;
			break;
		}
		case 0x45: // Display - LYC
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display lyc" << std::endl;
#endif
			display.lyc = value;
			break;
		}
		case 0x46: // DMA Tranfer
		{
#ifdef _DEBUG
			std::cout << "Initiated a DMA transfer from " << hex<uint16_t>(value << 8) << std::endl;
#endif
			DMATranfer(value);
			break;
		}
		case 0x47: // Display - Background palette
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display bg palette" << std::endl;
#endif
			display.bg_palette = value;
			break;
		}
		case 0x48: // Display - OBJ Palette 0
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display obj 0 palette" << std::endl;
#endif
			display.obj_palette0 = value;
			break;
		}
		case 0x49: // Display - OBJ Palette 1
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display obj 1 palette" << std::endl;
#endif
			display.obj_palette1 = value;
			break;
		}
		case 0x4A: // Display - Window Y
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display window y" << std::endl;
#endif
			display.winPosY = value;
			break;
		}
		case 0x4B: // Display - Window X
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to display window x" << std::endl;
#endif
			display.winPosX = value;
			break;
		}
		case 0x4D: // CGB - Speed Switch
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to CGB speed switch" << std::endl;
#endif
			speed_switch = value;
			break;
		}
		case 0x50: // DMG ROM disable
		{
			std::cout << "Startup A value : " << hex<uint8_t>(regs.A) << std::endl;
			ram.bootstrap = false;
			break;
		}
		case 0x76: case 0x77: case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
		{
			break;
		}
		case 0xFF: // Interrupt Enable
		{
#ifdef _DEBUG
			std::cout << "Wrote " << hex<uint8_t>(value) << " to interrupt enable" << std::endl;
#endif
			int_enable = value;
			break;
		}
		default:
		{
			std::cerr << "Unimplemented IO port write " << hex<uint8_t>(port) << std::endl;
			std::ostringstream sstream;
			sstream << "Warning, writing " << hex<uint8_t>(value) << " to IO port " << hex<uint8_t>(port);
			throw IOPortNotImplemented(sstream.str());
		}
		}
	}

	uint8_t OnIORead(uint8_t port)
	{
#ifdef _DEBUG
		if(port != 0x00 && port != 0x01 && port != 0x02 && port != 0x44)
			std::cout << "Reading from IO port " << hex<uint8_t>(port) << std::endl;
#endif
		switch (port)
		{
		case 0x00: // Keypad
		{
			uint8_t value = joypad.joypad;
			//std::cout << "--------------- INPUT " << hex<uint8_t>(value) << std::endl;
			return value;
		}
		case 0x01: // Serial data
			return serial.data;
		case 0x02: // Serial transfer control
			return serial.control;
		case 0x03: // UNUSED REGISTER
			return unused_regsiter_03;
		case 0x04: // Timer divider
			return timer.divider;
		case 0x05: // Timer counter
			return timer.counter;
		case 0x06: // Timer modulo
			return timer.modulo;
		case 0x07: // Timer control
			return timer.control;
		case 0x08: // UNUSED REGISTER
			return unused_regsiter_08;
		case 0x0F: // Interrupt flag
			return int_flag;
		case 0x11: // Sound - Channel 1 Sound lengh
			return sound.c1_duty;
		case 0x14: // Sound - Channel 1 Frequency High
			return sound.c1_frequency.upper;
		case 0x16: // Sound - Channel 2 Sound length
			return sound.c2_duty;
		case 0x18: // Sound - Channel 2 Frequency Low
			return 0; // Write only
			//return sound.c2_frequency.lower;
		case 0x19: // Sound - Channel 2 Frequency High
			return sound.c2_frequency.upper;
		case 0x1E: // Sound - Channel 3 Frequency High
			return sound.c3_frequency.upper;
		case 0x1C: // Sound - Channel 3 Output Level
			return sound.c3_output_level;
		case 0x23: // Sound - Channel 4 Counter consecutive
			return sound.c4_counter;
		case 0x24: // Sound - Channel control
			return sound.channel_control;
		case 0x25: // Sound - Sound output select
			return sound.sound_output;
		case 0x26: // Sound - Master control
			return sound.master_control;
		case 0x40: // Display - LCD Control
			return display.control;
		case 0x41: // Display - LCD Status
			return display.status;
		case 0x42: // Display - Scroll Y
			return display.scrollY;
		case 0x43: // Display - Scroll X
			return display.scrollX;
		case 0x44: // Display - LY
			return display.ly;
		case 0x47: // Display - Background palette
			return display.bg_palette;
		case 0x48: // Display - OBJ Palette 0
			return display.obj_palette0;
		case 0x49: // Display - OBJ Palette 1
			return display.obj_palette1;
		case 0x4A: // Display - Window Y
			return display.winPosY;
		case 0x4B: // Display - Window X
			return display.winPosX;
		case 0x4D: // CGB - Speed Switch
			return speed_switch;
		case 0xFF: // Interrupt master flag
			return (uint8_t)interruptsEnabled;
		default:
		{
			std::cerr << "Unimplemented IO port read " << hex<uint8_t>(port) << std::endl;
			std::ostringstream sstream;
			sstream << "Warning, reading from unimplemented IO port " << hex<uint8_t>(port);
			throw IOPortNotImplemented(sstream.str());
		}
		}
		return 0;
	}

	void interrupt(uint8_t value)
	{
		switch (value)
		{
		case 0x40: // V-Blank interrupt
			int_flag.vblank = 1;
			break;
		case 0x48: // LCD STAT interrupt
			int_flag.lcd_stat = 1;
			break;
		case 0x50: // Timer interrupt
			int_flag.timer = 1;
			break;
		default:
		{
			std::cerr << "ERR: INVALID INTERRUPT " << hex<uint8_t>(value) << std::endl;
			std::ostringstream sstream;
			sstream << "Invalid interrupt value: " << hex<uint8_t>(value);
			throw std::runtime_error(sstream.str());
		}
		}
	}

private:
	bool halfcarry8(uint8_t a, uint8_t b)
	{
		return (((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10;
	}

	bool halfcarry_sub(uint8_t a, uint8_t b)
	{
		return (a & 0x0F) < (b & 0x0F);
	}
	
	bool halfcarry16(uint16_t a, uint16_t b)
	{
		return halfcarry8(((uint8_t*)(&a))[1], ((uint8_t*)(&b))[1]);
	}

	bool halfcarryM(uint16_t a, uint8_t b)
	{
		return halfcarry8(((uint8_t*)(&a))[1], b);
	}

	void checkInterrupts()
	{
		if (!halted && !interruptsEnabled)
			return;
		if (int_enable.vblank && int_flag.vblank)
		{
			if (interruptsEnabled)
			{
				int_flag.vblank = false;
				executeInterrupt(0x40);
			}
			if (halted)
				halted = false;
		}
		else if (int_enable.lcd_stat && int_flag.lcd_stat)
		{
			if (interruptsEnabled)
			{
				int_flag.lcd_stat = false;
				executeInterrupt(0x48);
			}
			if (halted)
				halted = false;
		}
		else if (int_enable.timer && int_flag.timer)
		{
			if (interruptsEnabled)
			{
				int_flag.timer = false;
				executeInterrupt(0x50);
			}
			if (halted)
				halted = false;
		}
	}

	void executeInterrupt(uint16_t value)
	{
#ifdef _DEBUG
		std::cout << "Executing interrupt " << hex<uint16_t>(value) << std::endl;
#endif
		interruptsEnabled = false;
		push(regs.PC);
		jump(value);
	}
};

