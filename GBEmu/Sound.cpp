/*  This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "stdafx.h"

#include "SDL.h"
#include <inttypes.h>
#include <cstdlib>
#include <algorithm>
#include <thread>
#include <chrono>
#include <cstring>
#include <cmath>

#include "Sound.h"

channel1::channel1(Sound * sound) : sound(sound)
{
}

void channel1::check_overflow()
{
    if (sweep_frequency > 2047)
        sound->master_control.sound1 = 0;
}

void channel1::on_trigger()
{
    sound->master_control.sound1 = 1;
    if (sound->c1_duty.length_data == 0)
        sound->c1_duty.length_data = 0x3F;
    envelope_volume = sound->c1_envelope.initial_volume;
    envelope_counter = sound->c1_envelope.length;

    sweep_frequency = sound->c1_frequency.value;
    sweep_counter = sound->c1_sweep.sweep_time;
}

void channel1::on_sweep_clock()
{
    if (sound->c1_sweep.sweep_time == 0
        || sound->c1_sweep.sweep_shift == 0) // Sweep off
        return;
    sweep_counter--;
    if (sweep_counter == 0)
    {
        sweep_counter = sound->c1_sweep.sweep_time;
        uint16_t value = sweep_frequency >> sound->c1_sweep.sweep_shift;
        if (sound->c1_sweep.sweep_mode)
            sweep_frequency -= value;
        else
            sweep_frequency += value;
        check_overflow();
        sound->c1_frequency.value = sweep_frequency;
    }
}

void channel1::on_length_clock()
{
    if (sound->master_control.sound1 && sound->c1_frequency.upper.counter)
    {
        sound->c1_duty.length_data--;
        if (sound->c1_duty.length_data == 0)
            sound->master_control.sound1 = 0;
    }
}

void channel1::on_envelope_clock()
{
    envelope_counter--;
    if (envelope_counter == 0)
    {
        envelope_counter = sound->c1_envelope.length;

        int8_t change = sound->c1_envelope.direction ? 0x01 : -0x01;
        uint8_t new_volume = envelope_volume + change;
        if (new_volume > 0x0F)
            return;
        envelope_volume = new_volume;
    }
}

uint8_t square_duties[4][8] = {
    { 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 0, 0, 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1, 1, 1, 0 },
};

void channel1::on_frequency_clock()
{
    duty_cycle = (duty_cycle + 1) & 0x07;
}

uint64_t channel1::get_period()
{
    return (2048 - sound->c1_frequency.value) * 4;
}

uint8_t channel1::get_sample()
{
    if (sound->c1_envelope.initial_volume == 0
        && sound->c1_envelope.direction == 0) // OFF mode
        sound->master_control.sound1 = 0;

    if (!sound->master_control.sound1)
        return 0x00;

    if (sound->c1_envelope.length == 0
        && sound->c1_envelope.initial_volume == 0
        && sound->c1_envelope.direction == 1) // Second mute mode
        return 0x00;

    uint8_t value = 0x00;

    if (square_duties[sound->c1_duty.wave_pattern_duty][duty_cycle])
        value = envelope_volume;
    else
        value = 0x00;

    return value;
}

channel2::channel2(Sound * sound) : sound(sound)
{
}

void channel2::on_trigger()
{
    sound->master_control.sound2 = 1;
    if (sound->c2_duty.length_data == 0)
        sound->c2_duty.length_data = 0x3F;
    envelope_volume = sound->c2_envelope.initial_volume;
    envelope_counter = sound->c2_envelope.length;
}

void channel2::on_length_clock()
{
    if (sound->master_control.sound2 && sound->c2_frequency.upper.counter)
    {
        sound->c2_duty.length_data--;
        if (sound->c2_duty.length_data == 0)
            sound->master_control.sound2 = 0;
    }
}

void channel2::on_envelope_clock()
{
    envelope_counter--;
    if (envelope_counter == 0)
    {
        envelope_counter = sound->c2_envelope.length;
        int8_t change = sound->c2_envelope.direction ? 0x01 : -0x01;
        uint8_t new_volume = envelope_volume + change;
        if (new_volume > 0x0F)
            return;
        envelope_volume = new_volume;
    }
}

void channel2::on_frequency_clock()
{
    duty_cycle = (duty_cycle + 1) & 0x07;
}

uint64_t channel2::get_period()
{
    return (2048 - sound->c2_frequency.value) * 4;
}

uint8_t channel2::get_sample()
{
    if (sound->c2_envelope.initial_volume == 0
        && sound->c2_envelope.direction == 0) // OFF mode
        sound->master_control.sound2 = 0;

    if (!sound->master_control.sound2)
        return 0x00;

    if (sound->c2_envelope.length == 0
        && sound->c2_envelope.initial_volume == 0
        && sound->c2_envelope.direction == 1) // Second mute mode
        return 0x00;

    uint8_t value = 0x00;

    if (square_duties[sound->c2_duty.wave_pattern_duty][duty_cycle])
        value = envelope_volume;
    else
        value = 0x00;

    return value;
}

channel3::channel3(Sound * sound) : sound(sound) {}

void channel3::on_trigger()
{
    if (!sound->c3_on)
        return;

    sound->master_control.sound3 = 1;
    if (sound->c3_sound_length == 0)
        sound->c3_sound_length = 0xFF;
    sample_index = 0;
}

void channel3::on_length_clock()
{
    if (sound->master_control.sound3 && sound->c3_frequency.upper.counter)
    {
        sound->c3_sound_length--;
        if (sound->c3_sound_length == 0)
            sound->master_control.sound3 = 0;
    }
}

void channel3::on_frequency_clock()
{
    sample_index = (sample_index + 1) % 32;
}

uint64_t channel3::get_period()
{
    return (2048 - sound->c3_frequency.value) * 2;
}

static const uint8_t shift_values[4] = { 4, 0, 1, 2 };

uint8_t channel3::get_sample()
{
    if (!sound->c3_on)
        sound->master_control.sound3 = 0;

    if (!sound->master_control.sound3)
        return 0x00;

    uint8_t sample_byte = sound->wave_pattern[sample_index / 2];
    uint8_t sample_value = sample_index & 0x01 ? sample_byte & 0xF : sample_byte >> 4;
    sample_value >>= shift_values[sound->c3_output_level >> 5];

    return sample_value;
}

channel4::channel4(Sound * sound) : sound(sound) {}

void channel4::on_trigger()
{
    sound->master_control.sound4 = 1;
    if (sound->c4_sound_length == 0)
        sound->c4_sound_length = 0x3F;
    envelope_volume = sound->c4_envelope.initial_volume;
    envelope_counter = sound->c4_envelope.length;
    lfsr = 0x7FFF;
}

void channel4::on_length_clock()
{
    if (sound->master_control.sound4 && sound->c4_counter.counter)
    {
        sound->c4_sound_length--;
        if (sound->c4_sound_length == 0)
            sound->master_control.sound4 = 0;
    }
}

void channel4::on_envelope_clock()
{
    envelope_counter--;
    if (envelope_counter == 0)
    {
        envelope_counter = sound->c4_envelope.length;
        int8_t change = sound->c4_envelope.direction ? 0x01 : -0x01;
        uint8_t new_volume = envelope_volume + change;
        if (new_volume > 0x0F)
            return;
        envelope_volume = new_volume;
    }
}

void channel4::on_frequency_clock()
{
    bool low_xor = xor_a ^ xor_b;
    lfsr >>= 1;
    high_set = low_xor;
    if (sound->c4_poly_counter.width)
        low_set = low_xor;
}

uint64_t noise_periods[8] = {
    8, 16, 32, 48, 64, 80, 96, 112
};

uint64_t channel4::get_period()
{
    return noise_periods[sound->c4_poly_counter.ratio] << (sound->c4_poly_counter.shift_frequency + 1);
}

uint8_t channel4::get_sample()
{
    if (sound->c4_envelope.initial_volume == 0
        && sound->c4_envelope.direction == 0) // OFF mode
        sound->master_control.sound4 = 0;

    if (!sound->master_control.sound4)
        return 0x00;

    if (sound->c4_envelope.length == 0
        && sound->c4_envelope.initial_volume == 0
        && sound->c4_envelope.direction == 1) // Second mute mode
        return 0x00;

    uint8_t value = 0x00;

    if (!xor_a)
        value = envelope_volume;
    else
        value = 0x00;

    return value;
}

void audio_callback(void * _sound, Uint8 * stream, int length);

void Sound::init()
{
    free_buffs = SDL_CreateSemaphore(BUF_COUNT - 1);

    uint64_t device_count = SDL_GetNumAudioDevices(0);
    if (device_count > 0)
    {
        for (uint64_t device = 0; device < device_count; device++)
            std::cout << "Audio device " << device << ": " << SDL_GetAudioDeviceName((int)device, 0) << std::endl;

        SDL_AudioSpec spec;
        spec.freq = 44100;
        spec.format = AUDIO_U8;
        spec.channels = 1;
        spec.samples = WRITE_BUF_SIZE;
        spec.callback = audio_callback;
        spec.userdata = this;

        SDL_AudioSpec previous_spec;

        audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, &previous_spec, 0);
        if (audio_device == 0)
            std::cout << "Could not open audio device: " << SDL_GetError() << std::endl;
        else
            SDL_PauseAudioDevice(audio_device, 0);
    }
    else
    {
        std::cout << "No audio device found :o" << std::endl;
    }
}

void audio_callback(void * _sound, Uint8 * stream, int length)
{
    Sound * sound = (Sound*)_sound;
    sound->audio_callback_(stream, length);
}

void Sound::audio_callback_(uint8_t * stream, int length)
{
    if (SDL_SemValue(free_buffs) < BUF_COUNT - 1) // There is at least one full buffer
    {
        std::memcpy(stream, sound_buffer + read_ptr, length);
        std::memset(sound_buffer + read_ptr, 0x00, length);
        read_ptr = (read_ptr + length) % (SOUND_BUF_SIZE);
        SDL_SemPost(free_buffs);
    }
    else
    {
        //printf("Buffer is empty, read %" PRIu64 " - write %" PRIu64 "\n", read_ptr, write_ptr);
        std::memset(stream, 0x00, length);
    }
}

#define CYCLES_PER_SAMPLE 4194304. / 44100.

void Sound::OnMachineCycle(uint64_t cycles)
{
    static uint64_t sample_counter = 0;
    static uint64_t cycle_counter = 0;
    for (uint64_t cycle = 0; cycle < 4 * cycles; cycle++)
    {
        cycle_counter++;

        chan1.on_cycle();
        if (c1_frequency.upper.initial)
        {
            chan1.on_trigger();
            c1_frequency.upper.initial = 0;
        }
        chan2.on_cycle();
        if (c2_frequency.upper.initial)
        {
            chan2.on_trigger();
            c2_frequency.upper.initial = 0;
        }
        chan3.on_cycle();
        if (c3_frequency.upper.initial)
        {
            chan3.on_trigger();
            c3_frequency.upper.initial = 0;
        }
        chan4.on_cycle();
        if (c4_counter.initial)
        {
            chan4.on_trigger();
            c4_counter.initial = 0;
        }
#if 1
        elapsed_cycle += 1.;
        if (elapsed_cycle >= CYCLES_PER_SAMPLE)
        {
            sample_counter++;
            write_sample(sample_audio());
            elapsed_cycle = std::fmod(elapsed_cycle, CYCLES_PER_SAMPLE);
        }
#endif
    }

#if 0
    if (cycle_counter % 9500 == 0)
        std::cout << cycle_counter << ": " << sample_counter << " audio samples taken" << std::endl;
#endif

#if 0
    for (uint_fast64_t cycle = 0; cycle < cycles; cycle++)
    {
        cycleCount++;
        if (cycleCount == 23) // Sample the sound every 23 machine cycles
        {
            uint8_t sample = sample_audio();
            write_sample(sample);
            cycleCount = 0;
        }
    }
#endif
}

void Sound::clockFrameCounter()
{
}

static const double period_440 = 2.27272727;

uint8_t Sound::sample_audio()
{
    if (!master_control.all_sounds)
        return 0x00;

    uint8_t c1_sample = chan1.get_sample();
    uint8_t c2_sample = chan2.get_sample();
    uint8_t c3_sample = chan3.get_sample();
    uint8_t c4_sample = chan4.get_sample();

    uint64_t so1 = 0, so2 = 0;
    if (sound_output.sound1_so1)
        so1 += c1_sample;
    if (sound_output.sound1_so2)
        so2 += c1_sample;
    if (sound_output.sound2_so1)
        so1 += c2_sample;
    if (sound_output.sound2_so2)
        so2 += c2_sample;
    if (sound_output.sound3_so1)
        so1 += c3_sample;
    if (sound_output.sound3_so2)
        so2 += c3_sample;
    if (sound_output.sound4_so1)
        so1 += c4_sample;
    if (sound_output.sound4_so2)
        so2 += c4_sample;

    so1 *= (channel_control.so1_level + 1);
    so2 *= (channel_control.so2_level + 1);

    if (so1 > 0xFF)
        so1 = 0xFF;
    if (so2 > 0xFF)
        so2 = 0xFF;

    uint8_t mono = (uint8_t)(((uint16_t)so1 + so2) / 2);
    if (mono > 0xFF)
        return 0xFF;

    return mono;
}

#if 0
uint8_t Sound::sample_audio()
{
    const double delta = 1000. / 44100;
    static uint64_t sample_count = 0;
    sample_count = (sample_count + 1) % (3 * 44100);
    //double time = sample_count * delta;
    static double time = 0;

    if (!master_control.all_sounds)
        return 0x80;

    //bool c2_mute = 
    uint8_t c2_sample = sample_c2(time);

    uint8_t so1 = 0, so2 = 0;
    if (sound_output.sound2_so1)
        so1 += c2_sample;
    if (sound_output.sound2_so2)
        so2 += c2_sample;

    so1 = (uint8_t)(so1 * channel_control.so1_level / 7.);
    so2 = (uint8_t)(so2 * channel_control.so2_level / 7.);

    return so1 + so2;
}
#endif

void Sound::write_sample(uint8_t sample)
{
#if 1
    //(sound_buffer + write_ptr)[write_index] = (sample / 2) + 0x80;
    (sound_buffer + write_ptr)[write_index] = sample;
    write_index++;
    if (write_index == WRITE_BUF_SIZE)
    {
        write_index = 0;
        write_ptr = (write_ptr + WRITE_BUF_SIZE) % (SOUND_BUF_SIZE);
        //printf("Currently %" PRIu32 " buffers free\n", SDL_SemValue(free_buffs));
        SDL_SemWait(free_buffs); // A free buffer was taken
    }
#endif
}
