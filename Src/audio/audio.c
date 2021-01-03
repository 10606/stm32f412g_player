
/**
  ******************************************************************************
  * @file    BSP/Src/audio.c 
  * @author  MCD Application Team
  * @brief   This example code shows how to use the audio feature in the 
  *          stm32412g_discovery driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include "util.h"
#include "mad.h"
#include "view.h"
#include "audio.h"
#include "main.h"
#include "FAT.h"
#include "stm32412g_discovery_audio.h"
#include "display.h"
#include "display_string.h"
#include "touchscreen.h"
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_it.h"
#include "usb_command_process.h"

#define str_time_size 100
uint32_t audio_freq[8] = {8000 ,11025, 16000, 22050, 32000, 44100, 48000, 96000};

extern view viewer;
old_touch_state touch_state = {0};
audio_ctl  buffer_ctl;


static void Audio_SetHint (void);
static uint32_t get_pcm_sound (file_descriptor * _file, uint8_t * buffer, uint32_t size);
AUDIO_ErrorTypeDef AUDIO_Start ();
uint8_t AUDIO_Process (void);
volatile uint8_t need_redraw = 0;

#define mp3_input_buffer_size 1536LU
#define mp3_frame_size 1152LU
struct 
{
    uint8_t buffer[mp3_input_buffer_size + MAD_BUFFER_GUARD];
    uint32_t size;
} mp3_input_buffer;

struct
{
    struct mad_stream stream;
    struct mad_frame frame;
    struct mad_synth synth;
} mad_data;

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

char seeked = 0;

void audio_init ()
{
    buffer_ctl.repeat_mode = 0;
    buffer_ctl.audio_freq = *(audio_freq + 5); /*AF_44K*/
    uint8_t status = 0;
    buffer_ctl.pause_status = 0; /* 0 when audio is running, 1 when Pause is on */
    buffer_ctl.volume = AUDIO_DEFAULT_VOLUME;
  
    Audio_SetHint();
  
    status = BSP_JOY_Init(JOY_MODE_GPIO);
  
    if (status != HAL_OK)
        display_string_c(0, 140, (uint8_t*)"Joystick init error", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
  
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, buffer_ctl.volume, buffer_ctl.audio_freq) != 0)
        display_string_c(0, 140, (uint8_t*)"Audio codec fail", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
    
}

void audio_destruct ()
{
    BSP_AUDIO_OUT_DeInit();
    deinit_mad();
}

void check_buttons ()
{
    JOYState_TypeDef JoyState = JOY_NONE;
    JoyState = BSP_JOY_GetState();
  
    if (JoyState == JOY_UP)
        joystick_state.pressed[joy_button_up] = 1;
    else
        joystick_state.pressed[joy_button_up] = 0;
      
    if (JoyState == JOY_DOWN)
        joystick_state.pressed[joy_button_down] = 1;
    else
        joystick_state.pressed[joy_button_down] = 0;

    if (JoyState == JOY_RIGHT)
        joystick_state.pressed[joy_button_right] = 1;
    else
        joystick_state.pressed[joy_button_right] = 0;

    if (JoyState == JOY_LEFT)
        joystick_state.pressed[joy_button_left] = 1;
    else
        joystick_state.pressed[joy_button_left] = 0;

    if (JoyState == JOY_SEL)
        joystick_state.pressed[joy_button_center] = 1;
    else
        joystick_state.pressed[joy_button_center] = 0;
}

void AudioPlay_demo ()
{ 
    if (AUDIO_Start() == AUDIO_ERROR_IO)
    {
        deinit_mad();
        return;
    }
  
    need_redraw = 0;
    display_view(&viewer);
    while (1)
    {
        AUDIO_Process();
        check_buttons();
      
        if (need_redraw)
        {
            need_redraw = 0;
            display_view(&viewer);
        }
        uint8_t need_redraw_nv = 0;
        process_view(&viewer, &need_redraw_nv);
        usb_process();
        touch_check(&touch_state, &viewer, &need_redraw_nv);
        need_redraw |= need_redraw_nv;
        
        if (BSP_SD_IsDetected() != SD_PRESENT)
        {
            deinit_mad();
            break;
        }
    }
}

static void Audio_SetHint (void)
{
    fill_rect(0, HEADBAND_HEIGHT, 240, 240 - HEADBAND_HEIGHT, LCD_COLOR_WHITE);
    fill_rect(0, 0, 240, HEADBAND_HEIGHT, LCD_COLOR_BLUE);
}


AUDIO_ErrorTypeDef AUDIO_Start ()
{
    uint32_t bytesread;

    init_mad();
    if (open_song(&viewer.pl, &buffer_ctl.audio_file))
    {
        display_string_c(0, 152, (uint8_t*)"Not opened", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
        return AUDIO_ERROR_IO;
    }
    get_length(&buffer_ctl.audio_file, &buffer_ctl.info);
    if (f_seek(&buffer_ctl.audio_file, buffer_ctl.info.offset))
        display_string_c(0, 152, (uint8_t*)"Not seeked", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);

    buffer_ctl.state = BUFFER_OFFSET_NONE;
    bytesread = get_pcm_sound(&buffer_ctl.audio_file,
                      &buffer_ctl.buff[0],
                      AUDIO_BUFFER_SIZE);
    if (bytesread > 0)
    {
        BSP_AUDIO_OUT_Play((uint16_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE);
        buffer_ctl.audio_state = AUDIO_STATE_PLAYING;      
        return AUDIO_ERROR_NONE;
    }
    return AUDIO_ERROR_IO;
}

typedef struct tik_t
{
    uint16_t min;
    uint16_t sec;
    uint16_t ms;
} tik_t;

void byte_to_time (tik_t * time, uint32_t value)
{
    if (value >= buffer_ctl.info.offset)
        value -= buffer_ctl.info.offset;
    else
        value = 0;

    uint32_t time_ms = 
        (float)(buffer_ctl.info.length) /
        (float)(buffer_ctl.audio_file.size - buffer_ctl.info.offset) *
        (float)(value);
    
    time->ms = time_ms % 1000;
    time->sec = (time_ms / 1000) % 60;
    time->min = time_ms / 1000 / 60;
    
    /*
    uint32_t divider = buffer_ctl.audio_freq;
    value = value / 2 / 2; // 16 bit (2 bytes)   2 channels
    time->min = (value / divider) / 60;
    time->sec = (value / divider) % 60;
    time->ms = (value % divider) * 1000 / divider;
    */
}

uint8_t AUDIO_Process (void)
{
    uint32_t bytesread;
    AUDIO_ErrorTypeDef error_state = AUDIO_ERROR_NONE;  
  
    switch (buffer_ctl.audio_state)
    {
        case AUDIO_STATE_PLAYING:
        //display time
        {
            tik_t cur_time;
            tik_t total_time;
      
            byte_to_time(&cur_time, current_position(&buffer_ctl.audio_file));
            byte_to_time(&total_time, buffer_ctl.audio_file.size);

            char str[str_time_size];
            snprintf
            (
                str, 
                str_time_size, 
                " %4u:%02u.%03u / %u:%02u.%03u ", 
                cur_time.min,
                cur_time.sec,
                cur_time.ms,
                total_time.min,
                total_time.sec,
                total_time.ms
            );
            display_string_center_c(0, 60, (uint8_t *)str, &Font12, LCD_COLOR_BLUE, LCD_COLOR_WHITE);
        }

        //end of song reached
        if (current_position(&buffer_ctl.audio_file) >= buffer_ctl.audio_file.size)
        {
            // play song again
            if (buffer_ctl.repeat_mode)
            {
                deinit_mad();
                init_mad();
                if (f_seek(&buffer_ctl.audio_file, buffer_ctl.info.offset)) 
                    display_string_c(0, 152, (uint8_t*)"Not seeked", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
                seeked = 1;
            }
            else //or next song
            {
                next_playlist(&viewer.pl);
                deinit_mad();
                init_mad();
                if (open_song(&viewer.pl, &buffer_ctl.audio_file))
                    display_string_c(0, 152, (uint8_t*)"Not opened", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
                seeked = 1;
                get_length(&buffer_ctl.audio_file, &buffer_ctl.info);
                if (f_seek(&buffer_ctl.audio_file, buffer_ctl.info.offset))
                    display_string_c(0, 152, (uint8_t*)"Not seeked", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
            }
            error_state = AUDIO_ERROR_EOF;
            display_view(&viewer);
        }

        /* 1st half buffer played; so fill it and continue playing from bottom*/
        if (buffer_ctl.state == BUFFER_OFFSET_HALF)
        {
            bytesread = get_pcm_sound(&buffer_ctl.audio_file,
                          &buffer_ctl.buff[0],
                          AUDIO_BUFFER_SIZE /2);
      
            if (bytesread > 0)
            { 
                buffer_ctl.state = BUFFER_OFFSET_NONE;
            }
            else
            {
                //TODO FIXME
                display_string_c(0, 152, (uint8_t*)"err_read", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
            }
        }
        /* 2nd half buffer played; so fill it and continue playing from top */    
        if (buffer_ctl.state == BUFFER_OFFSET_FULL)
        {
            bytesread = get_pcm_sound(&buffer_ctl.audio_file,
                          &buffer_ctl.buff[AUDIO_BUFFER_SIZE /2],
                          AUDIO_BUFFER_SIZE /2);
            if (bytesread > 0)
            {
                buffer_ctl.state = BUFFER_OFFSET_NONE;
            }
            else
            {
                //TODO FIXME
                display_string_c(0, 152, (uint8_t*)"err_read", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
            }
        }
        break;
    
    default:
        error_state = AUDIO_ERROR_NOTREADY;
        break;
    }
    return (uint8_t)error_state;
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


static uint32_t get_pcm_sound (file_descriptor * _file, uint8_t * pbuf, uint32_t NbrOfData)
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
        else if ((mad_data.stream.bufend - mad_data.stream.buffer) < mp3_input_buffer_size)
            keep = mad_data.stream.bufend - mad_data.stream.buffer;
        else
            keep = mp3_input_buffer_size - mp3_frame_size;

        if (seeked)
        {
            keep = 0;
            seeked = 0;
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

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
    if (buffer_ctl.audio_state == AUDIO_STATE_PLAYING)
    {
        buffer_ctl.state = BUFFER_OFFSET_FULL;
    }
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
    if (buffer_ctl.audio_state == AUDIO_STATE_PLAYING)
    {
        buffer_ctl.state = BUFFER_OFFSET_HALF;
    }
}

void BSP_AUDIO_OUT_Error_CallBack(void)
{
    display_string_c(0, 120, (uint8_t*)"    DMA error", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);

    if (BSP_PB_GetState(BUTTON_WAKEUP) != RESET)
        return;
}

