#pragma once

class Sound;

class channel1
{
private:
    Sound * sound;

    uint8_t sweep_counter;
    uint16_t sweep_frequency;
    uint8_t duty_cycle;
    uint8_t envelope_counter;
    uint8_t envelope_volume;

    void check_overflow();

public:
    channel1() : sound(nullptr) { assert(false); }
    channel1(Sound * sound);

    void on_trigger();

    void on_sweep_clock();
    void on_length_clock();
    void on_envelope_clock();

    void on_frequency_clock();
    uint64_t get_period();

    uint8_t get_sample();
};

class channel2
{
private:
    Sound * sound;

    uint8_t duty_cycle;
    uint8_t envelope_counter;
    uint8_t envelope_volume;

public:
    channel2() : sound(nullptr) { assert(false); }
    channel2(Sound * sound);

    void on_trigger();

    void on_sweep_clock() {}
    void on_length_clock();
    void on_envelope_clock();

    void on_frequency_clock();
    uint64_t get_period();

    uint8_t get_sample();
};

class channel3
{
private:
    Sound * sound;

    uint16_t sample_index = 0;
public:
    channel3() : sound(nullptr) { assert(false); }
    channel3(Sound * sound);

    void on_trigger();

    void on_sweep_clock() {}
    void on_length_clock();
    void on_envelope_clock() {}

    void on_frequency_clock();
    uint64_t get_period();

    uint8_t get_sample();
};

template <typename ChanType>
class Channel : public ChanType
{
private:
	ChanType chan;

	uint64_t cycle_counter;
	uint64_t frequency_timer;
	uint8_t frame_sequencer;

	void on_frame_clock()
    {
        frame_sequencer = (frame_sequencer + 1) & 0x07; // Counts from 0 to 7
        if (frame_sequencer % 2 == 0)
            chan.on_length_clock();
        if (frame_sequencer % 4 == 2)
            chan.on_sweep_clock();
        if (frame_sequencer == 7)
            chan.on_envelope_clock();
    }

public:
    Channel(Sound * sound) : chan(sound) {}

    void on_cycle()
    {
        cycle_counter++;
        if (cycle_counter == 8192)
        {
            cycle_counter = 0;
            on_frame_clock();
        }
        frequency_timer--;
        if (frequency_timer == 0)
        {
            chan.on_frequency_clock();
            frequency_timer = chan.get_period();
        }
    }

    void on_trigger()
    {
        chan.on_trigger();
        frequency_timer = chan.get_period();
    }

    uint8_t get_sample() {
        return chan.get_sample();
    }
};

#define WRITE_BUF_SIZE 1024
#define BUF_COUNT 4
#define SOUND_BUF_SIZE BUF_COUNT * WRITE_BUF_SIZE

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
			return this->value & 0xC0;
		}
	};
	static_assert(sizeof(_wave_duty) == 1, "Wave pattern duty register not 1 byte");

	struct _volume_envelope
	{
		union {
			uint8_t value;
#pragma pack(push, 1)
			struct {
				uint8_t length : 3;
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
			this->value = (value & 0x80) | (this->value & 0x0F); return *this;
		}
		operator uint8_t() const
		{
			return this->value & 0x8F;
		}
	};
	static_assert(sizeof(_master_control) == 1, "Master control struct is not 1 byte");

	SDL_AudioDeviceID audio_device;
	uint8_t sound_buffer[SOUND_BUF_SIZE];
    uint64_t read_ptr = 0;
    uint64_t write_ptr = 0;
	uint64_t write_index = 0;
    SDL_semaphore * free_buffs;
	friend void audio_callback(void * _sound, Uint8 * stream, int length);

    double elapsed_cycle = 0;
	uint64_t frame_cycles = 0;
	void clockFrameCounter();
	uint8_t frame_counter = 0;
	uint_fast8_t cycleCount = 0;
	uint8_t sample_audio();
    void audio_callback_(uint8_t * stream, int length);

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

    Channel<channel1> chan1;
    Channel<channel2> chan2;
    Channel<channel3> chan3;

	Sound() : chan1(this), chan2(this), chan3(this) {}

	void OnMachineCycle(uint64_t cycles);
	void init();
	void write_sample(uint8_t sample);
};

