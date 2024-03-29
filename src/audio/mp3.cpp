#include "mp3.h"

#include "FAT.h"
#include "lcd_display.h"
#include "stm32412g_discovery_audio.h"
#include "display_error.h"
#include "audio.h"
#include <algorithm>
#include <string.h>
#include <stdint.h>

mp3_input_buffer_t mp3_input_buffer;
mad_data_t mad_data;

void init_mad ()
{
    mad_stream_init(&mad_data.stream);
    mad_synth_init(&mad_data.synth);
    mad_frame_init(&mad_data.frame);
}

void deinit_mad ()
{
    mad_synth_finish(&mad_data.synth);
    mad_frame_finish(&mad_data.frame);
    mad_stream_finish(&mad_data.stream);
}

void reuse_mad ()
{
    mad_stream_reuse(&mad_data.stream);
    mad_frame_reuse(&mad_data.frame);
    mad_synth_finish(&mad_data.synth);
    mad_synth_init(&mad_data.synth);
}

static uint32_t get_all_data (file_descriptor * _file, uint8_t * buffer, uint32_t size)
{
    uint32_t total_read = 0;
    ret_code ret = _file->read_all(buffer, size, &total_read);
    if (!ret) [[likely]]
        return total_read;
    else if (ret == err::eof_file) [[unlikely]]
        return 0;
    else
    {
        display::error("error read");
        return 0;
    }
}

inline int16_t scale (mad_fixed_t sample) 
{
     if (sample >= MAD_F_ONE)
         sample = MAD_F_ONE - 1;
     else if (sample < -MAD_F_ONE)
         sample = -MAD_F_ONE;
     return sample >> (MAD_F_FRACBITS + 1 - 16);
}

uint32_t fill_buffer
(
    struct mad_header const * header, 
    struct mad_pcm * pcm,
    uint8_t * buff, //[pcm->length * 4] //4608
    uint16_t pcm_length_max
) 
{
    if (pcm->samplerate != audio_ctl.audio_freq)
    {
        BSP_AUDIO_OUT_SetFrequency(pcm->samplerate);
        audio_ctl.audio_freq = pcm->samplerate;
    }
    mad_fixed_t const * left_ch = pcm->samples[0];
    mad_fixed_t const * right_ch = pcm->samples[1];
    uint32_t samples = std::min(pcm->length, pcm_length_max);
    if (pcm->channels == 2)
    {
        for (size_t cur_sample = 0; cur_sample != samples; ++cur_sample) 
        {
            int16_t sample;
            sample = scale(*(left_ch++));
            buff[cur_sample * 4    ] = ((sample >> 0) & 0xff);
            buff[cur_sample * 4 + 1] = ((sample >> 8) & 0xff);
            sample = scale(*(right_ch++));
            buff[cur_sample * 4 + 2] = ((sample >> 0) & 0xff);
            buff[cur_sample * 4 + 3] = ((sample >> 8) & 0xff);
        }
    } 
    else if (pcm->channels == 1) 
    {
        for (size_t cur_sample = 0; cur_sample != samples; ++cur_sample) 
        {
            int16_t sample;
            sample = scale(*(left_ch++));
            buff[cur_sample * 4    ] = ((sample >> 0) & 0xff);
            buff[cur_sample * 4 + 1] = ((sample >> 8) & 0xff);
            buff[cur_sample * 4 + 2] = ((sample >> 0) & 0xff);
            buff[cur_sample * 4 + 3] = ((sample >> 8) & 0xff);
        }
    }
    return samples * 4;
}


uint32_t get_pcm_sound (file_descriptor * _file, uint8_t * pbuf, uint32_t NbrOfData)
{
    if (_file->is_fake())
    {
        memset(pbuf, 0, NbrOfData);
        return NbrOfData;
    }

    if (audio_ctl.pause_status == pause_status_t::pause)
        return 0;
    
    uint32_t total_read = 0;
    uint32_t frames = 0;

    while (frames < NbrOfData)
    {
        uint32_t amount_safe_from_buffer = 0;
        if (mad_data.stream.next_frame != NULL)
            amount_safe_from_buffer = mad_data.stream.bufend - mad_data.stream.next_frame;
        else if ((mad_data.stream.bufend - mad_data.stream.buffer) < (long)mp3_input_buffer_size)
            amount_safe_from_buffer = mad_data.stream.bufend - mad_data.stream.buffer;
        else
            amount_safe_from_buffer = mp3_input_buffer_size - mp3_frame_size;

        if (audio_ctl.seeked)
        {
            amount_safe_from_buffer = 0;
            audio_ctl.seeked = 0;
        }
        
        if (amount_safe_from_buffer == mp3_input_buffer_size)
            amount_safe_from_buffer = mp3_input_buffer_size - mp3_frame_size;
        
        if (amount_safe_from_buffer)
        {
            mp3_input_buffer_t tmp_buffer;
            memcpy(tmp_buffer.buffer, mad_data.stream.bufend - amount_safe_from_buffer, amount_safe_from_buffer);
            memcpy(mp3_input_buffer.buffer, tmp_buffer.buffer, amount_safe_from_buffer);
            // memmove(mp3_input_buffer.buffer, mad_data.stream.bufend - amount_safe_from_buffer, amount_safe_from_buffer);
        }

        uint32_t rb = get_all_data(_file, mp3_input_buffer.buffer + amount_safe_from_buffer, mp3_input_buffer_size - amount_safe_from_buffer);
        mp3_input_buffer.size = amount_safe_from_buffer + rb;
        total_read += rb;

        if (rb + amount_safe_from_buffer < mp3_input_buffer_size)
        {
            uint32_t tail_size = std::min((uint32_t)MAD_BUFFER_GUARD, mp3_input_buffer_size - rb - amount_safe_from_buffer);
            memset(mp3_input_buffer.buffer + amount_safe_from_buffer + rb, 0, tail_size);
            amount_safe_from_buffer += tail_size;
        }

        mad_stream_buffer(&mad_data.stream, mp3_input_buffer.buffer, rb + amount_safe_from_buffer);

        uint32_t tries = 0;
        while ((frames < NbrOfData) && (tries < 5))
        {
            if (mad_frame_decode(&mad_data.frame, &mad_data.stream)) 
            {
                if (MAD_RECOVERABLE(mad_data.stream.error))
                {
                    tries++;
                    continue;
                }
                else if (mad_data.stream.error == MAD_ERROR_BUFLEN)
                {
                    break;
                }
                else
                {
                    reuse_mad();
                    audio_ctl.seeked = 1;
                    goto break_for;
                }
            }
            mad_synth_frame(&mad_data.synth, &mad_data.frame);
            frames += fill_buffer(&mad_data.frame.header, &mad_data.synth.pcm, pbuf + frames, NbrOfData - frames);
        }
        
        if (rb == 0)
            goto break_for;
    }
    break_for:
    return total_read;
}

