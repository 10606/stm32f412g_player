#include "mp3.h"

#include <string.h>
#include <stdint.h>
#include "audio.h"
#include "FAT.h"
#include "display.h"
#include "display_string.h"
#include "stm32412g_discovery_audio.h"
#include "util.h"

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

static uint32_t get_data (file_descriptor * _file, uint8_t * buffer, uint32_t size)
{
    uint32_t total_read = 0;
    uint32_t ret;
    uint32_t tried = 2;
    while ((ret = f_read(_file, buffer, size, &total_read)) && (tried))
    {
        if (ret == eof_file)
        {
            return 0;
        }
        --tried;
        display_string_c(0, 120, (uint8_t*)"    Error read", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
    }
    return total_read;
}

static uint32_t get_all_data (file_descriptor * _file, uint8_t * buffer, uint32_t size)
{
    uint32_t total_read = 0;
    while (total_read != size)
    {
        uint32_t cnt_read = get_data(_file, buffer + total_read, size - total_read);
        if (cnt_read == 0)
        {
            return total_read;
        }
        total_read += cnt_read;
    }
    return total_read;
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
    uint32_t pcm_length_max
) 
{
    if (pcm->samplerate != buffer_ctl.audio_freq)
    {
        BSP_AUDIO_OUT_SetFrequency(pcm->samplerate);
        buffer_ctl.audio_freq = pcm->samplerate;
    }
    mad_fixed_t const * left_ch = pcm->samples[0];
    mad_fixed_t const * right_ch = pcm->samples[1];
    uint32_t samples = min(pcm->length, pcm_length_max);
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
    return pcm->length * 4;
}


uint32_t get_pcm_sound (file_descriptor * _file, uint8_t * pbuf, uint32_t NbrOfData)
{
    if (buffer_ctl.pause_status)
        return 0;
    
    uint32_t total_read = 0;
    uint32_t frames = 0;

    while (frames < NbrOfData)
    {
        uint32_t keep = 0;
        if ((mad_data.stream.next_frame != NULL) && (mad_data.stream.error == MAD_ERROR_NONE))
            keep = mad_data.stream.bufend - mad_data.stream.next_frame;
        else if (mad_data.stream.error != MAD_ERROR_BUFLEN)
            keep = 0; //FIXME
        else if (mad_data.stream.next_frame != NULL)
            keep = mad_data.stream.bufend - mad_data.stream.next_frame;
        else if ((mad_data.stream.bufend - mad_data.stream.buffer) < (long)mp3_input_buffer_size)
            keep = mad_data.stream.bufend - mad_data.stream.buffer;
        else
            keep = mp3_input_buffer_size - mp3_frame_size;

        if (buffer_ctl.seeked)
        {
            keep = 0;
            buffer_ctl.seeked = 0;
        }
        
        if (keep == mp3_input_buffer_size)
            keep = mp3_input_buffer_size - mp3_frame_size;
        
        if (keep)
            memmove(mp3_input_buffer.buffer, mad_data.stream.bufend - keep, keep);

        uint32_t rb = get_all_data(_file, mp3_input_buffer.buffer + keep, mp3_input_buffer_size - keep);
        mp3_input_buffer.size = keep + rb;
        total_read += rb;

        if (rb + keep < mp3_input_buffer_size)
        {
            memset(mp3_input_buffer.buffer + keep + rb, 0, MAD_BUFFER_GUARD);
            keep += MAD_BUFFER_GUARD;
        }

        mad_stream_buffer(&mad_data.stream, mp3_input_buffer.buffer, rb + keep);

        uint32_t tries = 0;
        while ((frames < NbrOfData) && (tries < 100))
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
                    deinit_mad();
                    init_mad();
                    goto break_for;
                }
            }
            mad_synth_frame(&mad_data.synth, &mad_data.frame);
            frames += fill_buffer(&mad_data.frame.header, &mad_data.synth.pcm, pbuf + frames, AUDIO_BUFFER_SIZE - frames);
        }
        
        if (rb == 0)
            goto break_for;
    }
    break_for:
    return total_read;
}

