#include "audio.hpp"
#include "core.hpp"
#include "melon_types.hpp"
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <cstdio>

using namespace Mln;

    typedef enum
    {
        WAVE_PCM = 1,
        WAVE_FLOAT = 3
    } WaveFormat;

    constexpr size_t SoundBufferNodeCapacity = 32;

    struct SoundBufferNode
    {
        SoundBufferNode* next;
        int count;
        SoundBuffer buffers[SoundBufferNodeCapacity];
    };

    struct RawSound
    {
        int sample_rate;
        int channels;
        int bytes_per_sample;
        SoundFormat format;

        uint8_t* buffer;
        size_t buffer_size;
    };

    static void OnSendAudioDataToDevice(ma_device *device, void *frames_out, const void *frames_in, ma_uint32 frame_count);
    Sound ConvertRawSound(RawSound raw_sound);
    SoundBuffer* TrackSoundBuffer(SoundBuffer buffer); // Stores SoundBuffer in gAudio.buffer_list
    void DebugWriteWaveFile(RawSound raw_sound);

    static struct {
        ma_device device;
        ma_mutex lock;
        SoundBufferNode buffer_list;
    } gAudio = {0};


    int InitAudio()
    {
        ma_device_config config  = ma_device_config_init(ma_device_type_playback);
        config.playback.format   = ma_format_f32;   // Set to ma_format_unknown to use the device's native format.
        config.playback.channels = 2;               // Set to 0 to use the device's native channel count.
        config.sampleRate        = 0;               // Set to 0 to use the device's native sample rate.
        config.dataCallback      = OnSendAudioDataToDevice;   // This function will be called when miniaudio needs more data.
        config.pUserData         = NULL;   // Can be accessed from the device object (device.pUserData).

        if (ma_device_init(NULL, &config, &gAudio.device) != MA_SUCCESS) {
            return -1;  // Failed to initialize the device.
        }

        ma_device_start(&gAudio.device);     // The device is sleeping by default so you'll need to start it manually.

        return 0;
    }


    static void OnSendAudioDataToDevice(ma_device *device, void *frames_out, const void *frames_in, ma_uint32 frame_count)
    {
        // We are just adding all of the playing sounds
        memset(frames_out, 0, frame_count * device->playback.channels * ma_get_bytes_per_sample(device->playback.format));

        ma_mutex_lock(&gAudio.lock);
        {
            SoundBufferNode* iterator = &gAudio.buffer_list;

            while(iterator != NULL)
            {
                for (int i = 0; i < iterator->count; i++)
                {
                    SoundBuffer* buffer = &iterator->buffers[i];
                    ASSERT(buffer != NULL, "There should not be any null buffers in the buffer_list");

                    if (!buffer->playing)
                    {
                        continue;
                    }


                    uint32_t availableFrames = (buffer->frame_count - buffer->frames_processed);

                    bool finished = false;
                    if (availableFrames > frame_count)
                    {
                        availableFrames = frame_count;
                    }
                    else
                    {
                        finished = true;
                    }

                    // Accumulate all sound sources
                    int bytes_per_frame = buffer->bytes_per_sample * buffer->channels;
                    uint8_t* data_frame_start = buffer->data + buffer->frames_processed * bytes_per_frame;
                    for (int i = 0; i < availableFrames * buffer->channels; i++) {
                        ((float*)frames_out)[i] += ((float*)data_frame_start)[i];
                    }

                    buffer->frames_processed += availableFrames;

                    if (finished)
                    {
                        buffer->playing = buffer->looping;
                        buffer->frames_processed = 0;
                    }

                }

                iterator = iterator->next;
            }
        }

        ma_mutex_unlock(&gAudio.lock);
    }


    void PlaySound(Sound sound)
    {
        ma_mutex_lock(&gAudio.lock);
        sound.buffer->playing = true;
        sound.buffer->frames_processed = 0;
        ma_mutex_unlock(&gAudio.lock);
    }


    Sound LoadSoundFromFileWave(const char *filepath)
    {
        size_t file_size = 0;
        uint8_t* file_content = Mln::LoadFileBinary(filepath, &file_size);
        if (!file_content)
        {
            return Sound{0};
        }

        Sound sound = LoadSoundFromMemoryWave(file_content, file_size);

        Mln::UnloadFileBinary(file_content);
        
        return sound;

    }


    #define BYTES_TO_INT32(bytes) ((uint32_t)(bytes)[0] << 0 | (uint32_t)(bytes)[1] << 8 | (uint32_t)(bytes)[2] << 16 | (uint32_t)(bytes)[3] << 24)
    #define BYTES_TO_INT16(bytes) ((uint16_t)(bytes)[0] << 0 | (uint16_t)(bytes)[1] << 8)
    #define HAS_BYTES(start, end, bytes) ((((start) + (bytes)) <= end))
    static bool CompareBytes(uint8_t** cursor, uint8_t* cursor_end, const uint8_t* value, size_t size);
    static bool ReadBytes(uint8_t** cursor, uint8_t* cursor_end, size_t size, void* result);


    Sound LoadSoundFromMemoryWave(uint8_t* data, size_t size)
    {
        uint8_t* cursor = data;
        uint8_t* end = data + size;

        if (!CompareBytes(&cursor, end, (const uint8_t*)"RIFF", 4))
        {
            PrintLog(LOG_ERROR, "Incorrect RIFF tag\n");
            return Sound{};
        }

        uint8_t filesize_bytes[4];
        if (!ReadBytes(&cursor, end, 4, filesize_bytes))
        {
            PrintLog(LOG_ERROR, "Could not read filesize hint\n");
            return Sound{};
        }
        int32_t filesize = BYTES_TO_INT32(filesize_bytes);
        if (filesize != size - 8)
        {
            PrintLog(LOG_ERROR, "Incorrect filesize hint, file may be corrupted\n");
            return Sound{};
        }

        if (!CompareBytes(&cursor, end, (const uint8_t*)"WAVE", 4))
        {
            PrintLog(LOG_ERROR, "Incorrect WAVE tag\n");
            return Sound{};
        }

        if (!CompareBytes(&cursor, end, (const uint8_t*)"fmt ", 4))
        {
            PrintLog(LOG_ERROR, "Incorrect fmt tag\n");
            return Sound{};
        }
        
        int32_t fmt_size = BYTES_TO_INT32(cursor);
        cursor += 4;

        if (fmt_size < 16)
        {
            PrintLog(LOG_ERROR, "Incorrect fmt size, must be at least 16 bytes\n");
        }
        
        uint8_t* format_cursor = cursor;

        WaveFormat audio_format = (WaveFormat)BYTES_TO_INT16(format_cursor);
        format_cursor += 2;
        if (audio_format != WAVE_PCM && audio_format != WAVE_FLOAT)
        {
            PrintLog(LOG_ERROR, "Could not recognise audio format\n");
        }

        int32_t nb_channels = BYTES_TO_INT16(format_cursor);
        format_cursor += 2;

        int32_t frequency = BYTES_TO_INT32(format_cursor);
        format_cursor += 4;

        int32_t bytes_per_second = BYTES_TO_INT32(format_cursor);
        format_cursor += 4;

        int32_t byte_per_block = BYTES_TO_INT16(format_cursor);
        format_cursor += 2;

        int32_t bits_per_sample = BYTES_TO_INT16(format_cursor);
        format_cursor += 2;

        cursor += fmt_size;


        if (!CompareBytes(&cursor, end, (const uint8_t*)"data", 4))
        {
            PrintLog(LOG_ERROR, "Incorrect data tag\n");
            return Sound{};
        }

        int32_t data_size = BYTES_TO_INT32(cursor);
        cursor += 4;

        RawSound raw_sound = {};
        raw_sound.buffer = cursor;
        raw_sound.buffer_size = data_size;
        raw_sound.bytes_per_sample = bits_per_sample / 8;
        raw_sound.format = audio_format == WAVE_PCM ? (SoundFormat)raw_sound.bytes_per_sample : SOUND_FORMAT_F32; // Assuming audio_format is actually valid or 0
        raw_sound.channels = nb_channels;
        raw_sound.sample_rate = frequency;

        return ConvertRawSound(raw_sound);
    }


    void UnloadSound(Sound* sound)
    {
        // TODO: Remove the sound buffer from the list then free the sound itself
        
        SoundBufferNode* iterator = &gAudio.buffer_list;
        SoundBufferNode* owner = NULL;
        int index = 0;
        while (iterator != NULL) 
        {
            for (int i = 0; i < iterator->count; i++) 
            {
                if (&iterator->buffers[i] == sound->buffer)
                {
                    owner = iterator;
                    index = i;
                    break;
                }
            }
            
            iterator = iterator->next;
        }
        
        if (!owner)
        {
            PrintLog(LOG_ERROR, "Could not find sound to be removed.\n");
            return;
        }

        free(sound->buffer->data);
        sound->buffer = NULL;
        
        // Delete by swap with end
        owner->buffers[index] = owner->buffers[owner->count - 1];
        owner->count -= 1;
    }


    #include <cstring>

    static bool CompareBytes(uint8_t** cursor, uint8_t* cursor_end, const uint8_t* value, size_t size)
    {
        if (!HAS_BYTES(*cursor, cursor_end, size))
        {
            return false;
        }
        
        bool is_equal = strncmp((const char*)*cursor, (const char*)value, size) == 0;

        *cursor += size;

        return is_equal;
    }

    static bool ReadBytes(uint8_t** cursor, uint8_t* cursor_end, size_t size, void* result)
    {
        if (!HAS_BYTES(*cursor, cursor_end, size))
        {
            return false;
        }

        memcpy(result, *cursor, size);
        *cursor += size;
        
        return true;
    }



    Sound ConvertRawSound(RawSound raw_sound)
    {
        ASSERT((int)ma_format_count == (int)SOUND_FORMAT_COUNT_, "Invalid mapping between sound formats");

        ma_format format_in = (ma_format) raw_sound.format;
        ma_uint32 frame_count_in = raw_sound.buffer_size / (raw_sound.channels * raw_sound.bytes_per_sample);

        ma_uint32 frame_count = (ma_uint32)ma_convert_frames(NULL, 0, gAudio.device.playback.format, gAudio.device.playback.channels, gAudio.device.sampleRate, NULL, frame_count_in, format_in, raw_sound.channels, raw_sound.sample_rate);
        if (frame_count == 0) 
        {
            PrintLog(LOG_ERROR, "SOUND: Failed to get frame count for format conversion\n");
        }



        uint8_t* converted_audio = (uint8_t*)malloc(frame_count * gAudio.device.playback.channels * ma_get_bytes_per_sample(gAudio.device.playback.format));
        memset(converted_audio, 0, frame_count * gAudio.device.playback.channels * ma_get_bytes_per_sample(gAudio.device.playback.format));


        frame_count = (ma_uint32)ma_convert_frames(converted_audio, frame_count, gAudio.device.playback.format, gAudio.device.playback.channels, gAudio.device.sampleRate, raw_sound.buffer, frame_count_in, format_in, raw_sound.channels, raw_sound.sample_rate);
        if (frame_count == 0)
        {
            PrintLog(LOG_ERROR, "SOUND: Failed format conversion\n");
        }

        SoundBuffer buffer = {};
        buffer.data = converted_audio;
        buffer.frame_count = frame_count;
        
        buffer.channels = gAudio.device.playback.channels;
        buffer.sample_rate = gAudio.device.sampleRate;
        buffer.bytes_per_sample = ma_get_bytes_per_sample(gAudio.device.playback.format);

        buffer.frames_processed = 0;
        buffer.playing = false;
        buffer.looping = false;
        
        SoundBuffer* tracked_buffer = TrackSoundBuffer(buffer);
        Sound result = {tracked_buffer};
        return result;
    }

    SoundBuffer* TrackSoundBuffer(SoundBuffer buffer)
    {
        ma_mutex_lock(&gAudio.lock);

        SoundBufferNode* free_node = NULL;
        SoundBufferNode* last = &gAudio.buffer_list;
        SoundBufferNode* iterator = &gAudio.buffer_list;
        while (free_node == NULL && iterator != NULL) {
            if (iterator->count < SoundBufferNodeCapacity)
            {
                // We found a node with free space
                free_node = iterator;
                break;
            }
            
            last = iterator;
            iterator = iterator->next;
        }


        if (!free_node)
        {
            free_node = (SoundBufferNode*) malloc(sizeof(SoundBufferNode));
            free_node->count = 0;
            free_node->next = NULL;
            last->next = free_node;
        }

        SoundBuffer* tracked_buffer = &free_node->buffers[free_node->count++];
        *tracked_buffer = buffer;

        ma_mutex_unlock(&gAudio.lock);


        return tracked_buffer;
    }


    void DebugWriteWaveFile(const char* filepath, RawSound raw_sound)
    {
    // [Master RIFF chunk] (12 bytes)
    //    FileTypeBlocID  (4 bytes) : Identifier « RIFF »  (0x52, 0x49, 0x46, 0x46)
    //    FileSize        (4 bytes) : Overall file size minus 8 bytes
    //    FileFormatID    (4 bytes) : Format = « WAVE »  (0x57, 0x41, 0x56, 0x45)

    // [Chunk describing the data format] (24 bytes)
    //    FormatBlocID    (4 bytes) : Identifier « fmt␣ »  (0x66, 0x6D, 0x74, 0x20)
    //    BlocSize        (4 bytes) : Chunk size minus 8 bytes, which is 16 bytes here  (0x10)
    //    AudioFormat     (2 bytes) : Audio format (1: PCM integer, 3: IEEE 754 float)
    //    NbrChannels     (2 bytes) : Number of channels
    //    Frequency       (4 bytes) : Sample rate (in hertz)
    //    BytePerSec      (4 bytes) : Number of bytes to read per second (Frequency * BytePerBloc).
    //    BytePerBloc     (2 bytes) : Number of bytes per block (NbrChannels * BitsPerSample / 8).
    //    BitsPerSample   (2 bytes) : Number of bits per sample

    // [Chunk containing the sampled data] (8 + data size)
    //    DataBlocID      (4 bytes) : Identifier « data »  (0x64, 0x61, 0x74, 0x61)
    //    DataSize        (4 bytes) : SampledData size
    //    SampledData

        FILE* debug_write = fopen(filepath, "wb");
        fwrite("RIFF", 4, 1, debug_write);

        uint32_t file_size = raw_sound.bytes_per_sample * raw_sound.buffer_size + 36;
        fwrite(&file_size, 4, 1, debug_write); // 44 + data size - 8 = 36 + data size
        fwrite("WAVE", 4, 1, debug_write);
        
        fwrite("fmt ", 4, 1, debug_write);
        uint32_t fmt_size_x = 16;
        fwrite(&fmt_size_x, 4, 1, debug_write);
        uint16_t audio_format_x = raw_sound.format == SOUND_FORMAT_F32 ? WAVE_FLOAT : WAVE_PCM;
        fwrite(&audio_format_x, 2, 1, debug_write);
        fwrite(&raw_sound.channels, 2, 1, debug_write);
        fwrite(&raw_sound.sample_rate, 4, 1, debug_write);
        uint16_t bytes_per_block_x = raw_sound.channels * raw_sound.bytes_per_sample;
        uint32_t bytes_per_second_x = bytes_per_block_x * raw_sound.sample_rate;
        uint16_t bits_per_sample_x = raw_sound.bytes_per_sample * 8;
        fwrite(&bytes_per_second_x, 4, 1, debug_write);
        fwrite(&bytes_per_block_x, 2, 1, debug_write);
        fwrite(&bits_per_sample_x, 2, 1, debug_write);

        fwrite("data", 4, 1, debug_write);
        uint32_t data_size_x = raw_sound.buffer_size;
        fwrite(&data_size_x, 4, 1, debug_write);

        fwrite(raw_sound.buffer, raw_sound.bytes_per_sample * raw_sound.channels, raw_sound.buffer_size, debug_write);
        
        fclose(debug_write);
    }

