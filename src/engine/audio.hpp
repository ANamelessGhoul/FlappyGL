#pragma once

#include <cstdint>
#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "melon_types.hpp"

namespace Mln
{
    typedef enum {
        SOUND_FORMAT_INVALID = 0,
        SOUND_FORMAT_U8,
        SOUND_FORMAT_S16,
        SOUND_FORMAT_S24,
        SOUND_FORMAT_S32,
        SOUND_FORMAT_F32,
        SOUND_FORMAT_COUNT_
    } SoundFormat;


    // NOTE: This contains the sound data converted to the device format
    struct SoundBuffer{
        int sample_rate;
        int bytes_per_sample;
        int channels;

        uint8_t* data;
        size_t frame_count;

        int frames_processed;

        bool playing;
        bool looping;
    };

    // NOTE: This is a handle to the underlying sound buffer
    struct Sound{
        SoundBuffer* buffer;
    };

    int InitAudio();

    Sound LoadSoundFromFileWave(const char* filepath);
    Sound LoadSoundFromMemoryWave(uint8_t* data, size_t size);

    void UnloadSound(Sound* sound);

    void PlaySound(Sound sound);
}

#endif // AUDIO_HPP