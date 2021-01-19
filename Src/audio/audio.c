#include "audio.h"

#include "util.h"
#include "view.h"
#include "mp3.h"
#include "main.h"
#include "FAT.h"
#include "stm32412g_discovery_audio.h"
#include "display.h"
#include "display_string.h"
#include "touchscreen.h"
#include "joystick.h"
#include <stdio.h>
#include <stdint.h>
#include "usb_command_process.h"

#define str_time_size 100


old_touch_state touch_state = {0};
audio_ctl  buffer_ctl;


uint32_t audio_process (void);
volatile uint8_t need_redraw = 0;


static inline void set_song_hint (void)
{
    fill_rect(0, HEADBAND_HEIGHT, 240, 240 - HEADBAND_HEIGHT, LCD_COLOR_WHITE);
    fill_rect(0, 0, 240, HEADBAND_HEIGHT, LCD_COLOR_BLUE);
}

static inline audio_error_t audio_start ()
{
    uint32_t bytesread;

    init_mad();
    if (open_song(&viewer))
    {
        display_string_c(0, 152, (uint8_t*)"not opened", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
        return AUDIO_ERROR_IO;
    }

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


void audio_init ()
{
    buffer_ctl.seeked = 0;
    buffer_ctl.repeat_mode = 0;
    buffer_ctl.audio_freq = 44100; /*AF_44K*/
    uint8_t status = 0;
    buffer_ctl.pause_status = 0; /* 0 when audio is running, 1 when Pause is on */
    buffer_ctl.volume = AUDIO_DEFAULT_VOLUME;
  
    set_song_hint();
  
    status = BSP_JOY_Init(JOY_MODE_GPIO);
  
    if (status != HAL_OK)
        display_string_c(0, 140, (uint8_t*)"joystick init error", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
  
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, buffer_ctl.volume, buffer_ctl.audio_freq) != 0)
        display_string_c(0, 140, (uint8_t*)"audio codec fail", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
    
}

void audio_destruct ()
{
    BSP_AUDIO_OUT_DeInit();
    deinit_mad();
}

void audio_play ()
{ 
    if (audio_start() == AUDIO_ERROR_IO)
    {
        deinit_mad();
        return;
    }
  
    need_redraw = 0;
    display_view(&viewer);
    while (1)
    {
        if (audio_process())
            break;
        check_buttons();
      
        if (need_redraw)
        {
            need_redraw = 0;
            display_view(&viewer);
        }
        uint8_t need_redraw_nv = 0;
        if (process_view(&viewer, &need_redraw_nv))
            break;
        if (usb_process())
            break;
        touch_check(&touch_state, &viewer, &need_redraw_nv);
        need_redraw |= need_redraw_nv;
        
        if (BSP_SD_IsDetected() != SD_PRESENT)
        {
            deinit_mad();
            break;
        }
    }
}

uint32_t audio_process ()
{
    uint32_t bytesread;
    audio_error_t error_state = AUDIO_ERROR_NONE;  
  
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
                uint32_t ret;
                if ((ret = f_seek(&buffer_ctl.audio_file, buffer_ctl.info.offset)))
                {
                    display_string_c(0, 152, (uint8_t *)"not seeked", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
                    return ret;
                }
                buffer_ctl.seeked = 1;
            }
            else //or next song
            {
                next_playlist(&viewer.pl);
                deinit_mad();
                init_mad();
                uint32_t ret;
                if ((ret = open_song(&viewer)))
                {
                    display_string_c(0, 152, (uint8_t *)"not opened", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
                    return ret;
                }
            }
            error_state = AUDIO_ERROR_EOF;
            display_view(&viewer);
        }

        uint8_t * buffer;
        /* 1st half buffer played; so fill it and continue playing from bottom*/
        if (buffer_ctl.state == BUFFER_OFFSET_HALF)
            buffer = buffer_ctl.buff;
        /* 2nd half buffer played; so fill it and continue playing from top */    
        if (buffer_ctl.state == BUFFER_OFFSET_FULL)
            buffer = buffer_ctl.buff + (AUDIO_BUFFER_SIZE / 2);

        if ((buffer_ctl.state == BUFFER_OFFSET_FULL) ||
            (buffer_ctl.state == BUFFER_OFFSET_HALF))
        {
            bytesread = get_pcm_sound(&buffer_ctl.audio_file, buffer, AUDIO_BUFFER_SIZE / 2);
            if (bytesread > 0)
                buffer_ctl.state = BUFFER_OFFSET_NONE;
            else
            {
                //TODO FIXME
                display_string_c(0, 152, (uint8_t *)"err_read", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);
            }
        }
        break;
    
    default:
        error_state = AUDIO_ERROR_NOTREADY;
        break;
    }
    return (uint8_t)error_state;
}


void BSP_AUDIO_OUT_TransferComplete_CallBack (void)
{
    if (buffer_ctl.audio_state == AUDIO_STATE_PLAYING)
        buffer_ctl.state = BUFFER_OFFSET_FULL;
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack (void)
{
    if (buffer_ctl.audio_state == AUDIO_STATE_PLAYING)
        buffer_ctl.state = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_Error_CallBack (void)
{
    display_string_c(0, 120, (uint8_t*)"    DMA error", &Font16, LCD_COLOR_WHITE, LCD_COLOR_RED);

    if (BSP_PB_GetState(BUTTON_WAKEUP) != RESET)
        return;
}

