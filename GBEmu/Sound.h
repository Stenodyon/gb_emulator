#pragma once
class Sound
{
private:
	struct _sweep_register
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t sweep_shift : 3;
				uint8_t sweep_mode : 1; // (0 = addition, 1 = subtraction)
				uint8_t sweep_time : 3;
			};
#pragma pack(pop)
		};

		_sweep_register& operator=(uint8_t value) {
			this->value = value; return *this;
		}
		operator uint8_t() const {
			return this->value;
		}
	};
	static_assert(sizeof(_sweep_register) == 1, "Sweep register not 1 byte");

	struct _wave_duty
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t length_data : 6;
				uint8_t wave_pattern_duty : 2;
			};
#pragma pack(pop)
		};

		_wave_duty& operator=(uint8_t value)
		{
			this->value = value; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	static_assert(sizeof(_wave_duty) == 1, "Wave pattern duty register not 1 byte");

	struct _volume_envelope
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t sweep_count : 3;
				uint8_t direction : 1;
				uint8_t initial_volume : 4;
			};
#pragma pack(pop)
		};

		_volume_envelope& operator=(uint8_t value)
		{
			this->value = value; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	static_assert(sizeof(_volume_envelope) == 1, "Volume envelope register not 1 byte");

	struct _frequency_hi
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t upper : 3;
				uint8_t unused : 3;
				uint8_t counter : 1;
				uint8_t initial : 1;
			};
#pragma pack(pop)
		};

		_frequency_hi& operator=(uint8_t value)
		{
			this->value = value; return *this;
		}
		operator uint8_t() const
		{
			return this->value & (0x1 << 6); // Only the counter is readable
		}
	};
	static_assert(sizeof(_frequency_hi) == 1, "Frequency hi register not 1 byte");

	struct _frequency
	{
		union {
			uint16_t value : 11;
#pragma pack(push, 1)
			struct {
				uint8_t lower;
				_frequency_hi upper;
			};
#pragma pack(pop)
		};
	};
	static_assert(sizeof(_frequency) == 2, "Frequency struct not 2 bytes");

	struct _poly_counter
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t ratio : 3;
				uint8_t counter_step : 1;
				uint8_t shift_frequency : 4;
			};
#pragma pack(pop)
		};

		_poly_counter& operator=(uint8_t value)
		{
			this->value = value; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	static_assert(sizeof(_poly_counter) == 1, "Polynomial counter struct is not 1 byte");

	struct _channel_control
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t so1_level : 3;
				uint8_t vin_to_so1 : 1;
				uint8_t so2_level : 3;
				uint8_t vin_to_so2 : 1;
			};
#pragma pack(pop)
		};

		_channel_control& operator=(uint8_t value)
		{
			this->value = value; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	static_assert(sizeof(_channel_control) == 1, "Channel control struct is not 1 byte");

	struct _sound_output
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t sound1_so1 : 1;
				uint8_t sound2_so1 : 1;
				uint8_t sound3_so1 : 1;
				uint8_t sound4_so1 : 1;
				uint8_t sound1_so2 : 1;
				uint8_t sound2_so2 : 1;
				uint8_t sound3_so2 : 1;
				uint8_t sound4_so2 : 1;
			};
#pragma pack(pop)
		};

		_sound_output& operator=(uint8_t value)
		{
			this->value = value; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	static_assert(sizeof(_sound_output) == 1, "Sound output struct is not 1 byte");
	
	struct _master_control
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t sound1 : 1;
				uint8_t sound2 : 1;
				uint8_t sound3 : 1;
				uint8_t sound4 : 1;
				uint8_t unused : 3;
				uint8_t all_sounds : 1;
			};
#pragma pack(pop)
		};

		_master_control& operator=(uint8_t value)
		{
			this->value = value & 0x80; return *this;
		}
		operator uint8_t() const
		{
			return this->value;
		}
	};
	static_assert(sizeof(_master_control) == 1, "Master control struct is not 1 byte");
public:
	_sweep_register c1_sweep;
	_wave_duty c1_duty, c2_duty;
	_volume_envelope c1_envelope, c2_envelope, c4_envelope;
	_frequency c1_frequency, c2_frequency, c3_frequency;
	uint8_t c3_on, c3_sound_length, c3_output_level, c4_sound_length;
	_poly_counter c4_poly_counter;
	_frequency_hi c4_counter;
	uint8_t wave_pattern[16];

	_channel_control channel_control;
	_sound_output sound_output;
	_master_control master_control;

	Sound() {}
};

