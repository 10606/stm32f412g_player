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
audio_ctl_t  audio_ctl;
volatile uint8_t need_redraw = 0;


static inline audio_error_t audio_start ()
{
    init_mad();
    if (open_song_not_found(&viewer, 0))
    {
        display_error("err open song at start");
        return AUDIO_ERROR_IO;
    }

    memset(audio_ctl.buff, 0, audio_buffer_size);
    BSP_AUDIO_OUT_Play((uint16_t*)&audio_ctl.buff[0], audio_buffer_size);
    audio_ctl.audio_state = AUDIO_STATE_PLAYING;      
    return AUDIO_ERROR_NONE;
}


void audio_init ()
{
    audio_ctl.seeked = 0;
    audio_ctl.repeat_mode = 0;
    audio_ctl.audio_freq = 44100; /*AF_44K*/
    audio_ctl.pause_status = 0; /* 0 when audio is running, 1 when Pause is on */
    audio_ctl.volume = audio_default_volume;
  
    display_song_hint();
    init_fake_file_descriptor(&audio_ctl.audio_file);
  
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, audio_ctl.volume, audio_ctl.audio_freq) != 0)
        display_error("err init audio codec");
    
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
        uint32_t ret;
        if ((ret = audio_process(&need_redraw)))
        {
            if ((ret != AUDIO_ERROR_EOF) &&
                (ret != AUDIO_ERROR_NOTREADY))
                ; //break;
        }
        check_buttons();
      
        if (need_redraw)
        {
            need_redraw = 0;
            display_view(&viewer);
        }
        uint8_t need_redraw_nv = 0;
        if ((ret = process_view(&viewer, &need_redraw_nv)))
        {
            if (ret != empty_playlist)
                ;//break;
        }
        if (usb_process(&viewer, &need_redraw_nv))
            ; //break;
        touch_check(&touch_state, &viewer, &need_redraw_nv);
        need_redraw |= need_redraw_nv;
        
        if (BSP_SD_IsDetected() != SD_PRESENT)
        {
            deinit_mad();
            break;
        }
    }
}

typedef struct tik_t
{
    uint16_t min;
    uint16_t sec;
    uint16_t ms;
} tik_t;

static void byte_to_time (tik_t * time, uint32_t value)
{
    if (audio_ctl.audio_file.size == 0)
    {
        time->ms = 0;
        time->sec = 0;
        time->min = 0;
        return;
    }

    if (value >= audio_ctl.info.offset)
        value -= audio_ctl.info.offset;
    else
        value = 0;

    uint32_t time_ms = 
        (float)(audio_ctl.info.length) /
        (float)(audio_ctl.audio_file.size - audio_ctl.info.offset) *
        (float)(value);
    
    time->ms = time_ms % 1000;
    time->sec = (time_ms / 1000) % 60;
    time->min = time_ms / 1000 / 60;
}


uint32_t audio_process ()
{
    uint32_t bytesread;
    audio_error_t error_state = AUDIO_ERROR_NONE;  
  
    switch (audio_ctl.audio_state)
    {
        case AUDIO_STATE_PLAYING:
        //display time
        {
            tik_t cur_time;
            tik_t total_time;
      
            byte_to_time(&cur_time, current_position(&audio_ctl.audio_file));
            byte_to_time(&total_time, audio_ctl.audio_file.size);

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
            display_string_center_c(0, 60, str, &Font12, LCD_COLOR_BLUE, LCD_COLOR_WHITE);
        }

        //end of song reached
        if (current_position(&audio_ctl.audio_file) >= audio_ctl.audio_file.size)
        {
            if (!is_fake_file_descriptor(&audio_ctl.audio_file))
            {
                deinit_mad();
                init_mad();
            }
            // play song again
            if (audio_ctl.repeat_mode)
            {
                uint32_t ret;
                if ((ret = f_seek(&audio_ctl.audio_file, audio_ctl.info.offset)))
                {
                    display_error("err seek");
                    return ret;
                }
                audio_ctl.seeked = 1;
            }
            else //or next song
            {
                uint32_t ret;
                if ((ret = next_playlist(&viewer.pl)))
                {
                    display_error("err get next song");
                    return ret;
                }
                if ((ret = open_song_not_found(&viewer, 0)))
                {
                    display_error("err open song");
                    return ret;
                }
            }
            error_state = AUDIO_ERROR_EOF;
            need_redraw |= 1;
        }

        uint8_t * buffer;
        /* 1st half buffer played; so fill it and continue playing from bottom*/
        if (audio_ctl.state == BUFFER_OFFSET_HALF)
            buffer = audio_ctl.buff;
        /* 2nd half buffer played; so fill it and continue playing from top */    
        if (audio_ctl.state == BUFFER_OFFSET_FULL)
            buffer = audio_ctl.buff + (audio_buffer_size / 2);

        if ((audio_ctl.state == BUFFER_OFFSET_FULL) ||
            (audio_ctl.state == BUFFER_OFFSET_HALF))
        {
            bytesread = get_pcm_sound(&audio_ctl.audio_file, buffer, audio_buffer_size / 2);
            if (bytesread > 0)
                audio_ctl.state = BUFFER_OFFSET_NONE;
            else
            {
                //ignore
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
    if (audio_ctl.audio_state == AUDIO_STATE_PLAYING)
        audio_ctl.state = BUFFER_OFFSET_FULL;
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack (void)
{
    if (audio_ctl.audio_state == AUDIO_STATE_PLAYING)
        audio_ctl.state = BUFFER_OFFSET_HALF;
}

void BSP_AUDIO_OUT_Error_CallBack (void)
{
    display_error("err audio out");
}

