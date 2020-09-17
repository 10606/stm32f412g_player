
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
#include "view.h"
#include "audio.h"
#include "main.h"
#include "FAT.h"
#include "stm32412g_discovery_audio.h"
#include "display.h"
#include "touchscreen.h"
#include <stdio.h>
#include <stdint.h>
#include "stm32f4xx_it.h"

#define str_time_size 100
uint32_t audio_freq[8] = {8000 ,11025, 16000, 22050, 32000, 44100, 48000, 96000};

extern view viewer;
old_touch_state touch_state = {0};
audio_ctl  buffer_ctl;

static JOYState_TypeDef JoyState = JOY_NONE;

static void Audio_SetHint (void);
static uint32_t GetData (file_descriptor * file, uint8_t *pbuf, uint32_t NbrOfData);
static uint32_t get_all_data (file_descriptor * _file, uint8_t *pbuf, uint32_t NbrOfData);
AUDIO_ErrorTypeDef AUDIO_Start ();
uint8_t AUDIO_Process (void);



void audio_init ()
{
    buffer_ctl.repeat_mode = 0;
    buffer_ctl.audio_freq_ptr = audio_freq + 5; /*AF_44K*/
    uint8_t status = 0;
    buffer_ctl.pause_status = 0; /* 0 when audio is running, 1 when Pause is on */
    buffer_ctl.volume = AUDIO_DEFAULT_VOLUME;
  
    Audio_SetHint();
  
    status = BSP_JOY_Init(JOY_MODE_GPIO);
  
    if (status != HAL_OK)
    {    
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE); 
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 100, (uint8_t *)"ERROR", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"Joystick init error", CENTER_MODE);
    }
  
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, buffer_ctl.volume, *buffer_ctl.audio_freq_ptr) != 0)
    {
        BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 100, (uint8_t *)"  AUDIO CODEC  FAIL ", CENTER_MODE);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)" Try to reset board ", CENTER_MODE);
    }
}

void audio_destruct ()
{
    BSP_AUDIO_OUT_DeInit();
}


void AudioPlay_demo ()
{ 
    //BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 150, (uint8_t *)viewer.pl.song.song_name, LEFT_MODE);
    buffer_ctl.audio_freq_ptr = audio_freq + 5; /*AF_44K*/

    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE); 
    if (AUDIO_Start() == AUDIO_ERROR_IO)
    {
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 125, (uint8_t *)"       ERROR READ    ", CENTER_MODE);
    }
  
    uint8_t need_redraw = 0;
    display_view(&viewer);
    while (1)
    {
        AUDIO_Process();
        JoyState = BSP_JOY_GetState();
      
        switch (JoyState)
        {
        case JOY_UP:
            joystick_state.pressed[joy_button_up] = 1;
            break;
          
        case JOY_DOWN:
            joystick_state.pressed[joy_button_down] = 1;
            break;

        case JOY_RIGHT:
            joystick_state.pressed[joy_button_right] = 1;
            break;

        case JOY_LEFT:
            joystick_state.pressed[joy_button_left] = 1;
            break;

        case JOY_SEL:
            joystick_state.pressed[joy_button_center] = 1;
            break;

        default:
            break;
        }
      
        if (need_redraw)
        {
            display_view(&viewer);
            need_redraw = 0;
        }
        process_view(&viewer, &need_redraw);
        touch_check (&touch_state, &viewer, &need_redraw);
        
        /*
        BSP_AUDIO_OUT_SetVolume(buffer_ctl.volume);
        BSP_AUDIO_OUT_SetFrequency(*buffer_ctl.audio_freq_ptr);
        
        case JOY_SEL:
            // Set Pause / Resume or Exit 
            HAL_Delay(200);
            if (BSP_JOY_GetState() == JOY_SEL)  // Long press on joystick selection button : Pause/Resume 
            {
                if (buffer_ctl.pause_status == 1)
                { // Pause is enabled, call Resume 
                    BSP_AUDIO_OUT_Resume();
                    buffer_ctl.pause_status = 0;
                    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"       PLAYING...     ", CENTER_MODE);
                } 
                else
                { // Pause the playback 
                    BSP_AUDIO_OUT_Pause();
                    buffer_ctl.pause_status = 1;
                    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 85, (uint8_t *)"       PAUSE  ...     ", CENTER_MODE);
                }
                BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"                      ", CENTER_MODE);
                HAL_Delay(200);
            }
            else  // Short press on joystick selection button : exit 
            {
                BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                AUDIO_Stop();
                return;
            }
            break;
        
        default:
            break;
        }
        */
    }
}

static void Audio_SetHint (void)
{
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), HEADBAND_HEIGHT);
}


AUDIO_ErrorTypeDef AUDIO_Start ()
{
    uint32_t bytesread;

    if (open_song(&viewer.pl, &buffer_ctl.audio_file))
    {
        BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
        return AUDIO_ERROR_IO;
    }
    buffer_ctl.audio_file_size = buffer_ctl.audio_file.size;

    buffer_ctl.state = BUFFER_OFFSET_NONE;
    bytesread = get_all_data(&buffer_ctl.audio_file,
                      &buffer_ctl.buff[0],
                      AUDIO_BUFFER_SIZE);
    if (bytesread > 0)
    {
        BSP_AUDIO_OUT_Play((uint16_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE);
        buffer_ctl.audio_state = AUDIO_STATE_PLAYING;      
        buffer_ctl.fptr = bytesread;
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
    uint32_t divider = *buffer_ctl.audio_freq_ptr;
    value = value / 2 / 2; // 16 bit (2 bytes)   2 channels
    time->min = (value / divider) / 60;
    time->sec = (value / divider) % 60;
    time->ms = (value % divider) * 1000 / divider;
}

uint8_t AUDIO_Process (void)
{
    uint32_t bytesread;
    AUDIO_ErrorTypeDef error_state = AUDIO_ERROR_NONE;  
  
    switch (buffer_ctl.audio_state)
    {
        case AUDIO_STATE_PLAYING:
        {
            tik_t cur_time;
            tik_t total_time;
      
            byte_to_time(&cur_time, buffer_ctl.fptr);
            byte_to_time(&total_time, buffer_ctl.audio_file_size);

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
            BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetFont(&Font12);
            BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize()- 180, (uint8_t *)str, CENTER_MODE);
        }

        if (buffer_ctl.fptr >= buffer_ctl.audio_file_size)
        {
            /* Play audio sample again ... */
            buffer_ctl.fptr = 0; 
            if (buffer_ctl.repeat_mode)
            {
                if (f_seek(&buffer_ctl.audio_file, 0)) //TODO repeat mode
                {
                    BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not seeked...", 0);
                }
            }
            else
            {
                next_playlist(&viewer.pl);
                if (open_song(&viewer.pl, &buffer_ctl.audio_file))
                {
                    BSP_LCD_DisplayStringAt(0, 152, (uint8_t*)"Not opened...", 0);
                }
                buffer_ctl.audio_file_size = buffer_ctl.audio_file.size;
            }
            error_state = AUDIO_ERROR_EOF;
            display_view(&viewer);
        }

        /* 1st half buffer played; so fill it and continue playing from bottom*/
        if (buffer_ctl.state == BUFFER_OFFSET_HALF)
        {
            bytesread = get_all_data(&buffer_ctl.audio_file,
                          &buffer_ctl.buff[0],
                          AUDIO_BUFFER_SIZE /2);
      
            if (bytesread > 0)
            { 
                buffer_ctl.state = BUFFER_OFFSET_NONE;
                buffer_ctl.fptr += bytesread; 
            }
        }
        /* 2nd half buffer played; so fill it and continue playing from top */    
        if (buffer_ctl.state == BUFFER_OFFSET_FULL)
        {
            bytesread = get_all_data(&buffer_ctl.audio_file,
                          &buffer_ctl.buff[AUDIO_BUFFER_SIZE /2],
                          AUDIO_BUFFER_SIZE /2);
            if (bytesread > 0)
            {
                buffer_ctl.state = BUFFER_OFFSET_NONE;
                buffer_ctl.fptr += bytesread;
            }
        }
        break;
    
    default:
        error_state = AUDIO_ERROR_NOTREADY;
        break;
    }
    return (uint8_t)error_state;
}

static uint32_t GetData (file_descriptor * _file, uint8_t * pbuf, uint32_t NbrOfData)
{
    uint32_t BytesRead = 0;
    uint32_t ret;
    while ((ret = f_read(_file, pbuf, NbrOfData, &BytesRead)))
    {
        if (ret == eof_file)
        {
            return 0;
        }
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 120, (uint8_t *)"       ERR READ", CENTER_MODE);
    }
    return BytesRead;
}

static uint32_t get_all_data (file_descriptor * _file, uint8_t *pbuf, uint32_t NbrOfData)
{
    uint32_t BytesRead = 0;
    while (BytesRead != NbrOfData)
    {
        uint32_t cnt_read = GetData(_file, pbuf + BytesRead, NbrOfData - BytesRead);
        if (cnt_read == 0)
        {
            return BytesRead;
        }
        BytesRead += cnt_read;
    }
    return BytesRead;
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
    BSP_LCD_SetBackColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, LINE(14), (uint8_t *)"       DMA  ERROR     ", CENTER_MODE);
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

    while (BSP_PB_GetState(BUTTON_WAKEUP) != RESET)
        return;
}

