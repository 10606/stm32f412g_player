#include "audio.h"

#include "view.h"
#include "lcd_display.h"
#include "stm32412g_discovery_audio.h"
#include "display_offsets.h"
#include "display_song.h"
#include "display_error.h"
#include "mp3.h"
#include <stdio.h>
#include <stdint.h>

size_t const str_time_size = 100;

audio_ctl_t  audio_ctl;

audio_ctl_t::audio_ctl_t () :
    audio_freq(44100),
    volume(70),
    pause_status(pause_status_t::play),
    repeat_mode(0),
    seeked(0),
    state(buffer_offset_none)
{}

ret_code audio_ctl_t::audio_init ()
{
    need_redraw = 1;
    display::song_hint();
    audio_file.init_fake();
  
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, volume, audio_freq) != 0)
        display::error("err init audio codec");
    
    init_mad();
    ret_code ret;
    ret = viewer.open_song();
    if (ret)
    {
        deinit_mad();
        display::error("err open song at start");
        return ret;
    }

    memset(buff, 0, audio_buffer_size);
    BSP_AUDIO_OUT_Play((uint16_t*)&buff[0], audio_buffer_size);
    return 0;
}

void audio_ctl_t::audio_destruct ()
{
    BSP_AUDIO_OUT_DeInit();
    deinit_mad();
}

void audio_ctl_t::byte_to_time (tik_t * time, uint32_t value) const
{
    if (audio_file.size == 0)
    {
        time->ms = 0;
        time->sec = 0;
        time->min = 0;
        return;
    }

    if (value >= info.offset)
        value -= info.offset;
    else
        value = 0;

    uint32_t time_ms = 
        (float)(info.length) /
        (float)(audio_file.size - info.offset) *
        (float)(value);
    
    time->ms = time_ms % 1000;
    time->sec = (time_ms / 1000) % 60;
    time->min = time_ms / 1000 / 60;
}

void audio_ctl_t::next_pcm_part ()
{
    uint8_t * buffer;
    // 1st half buffer played; so fill it and continue playing from bottom
    if (state == buffer_offset_half)
        buffer = buff;
    // 2nd half buffer played; so fill it and continue playing from top
    if (state == buffer_offset_full)
        buffer = buff + (audio_buffer_size / 2);

    if ((state == buffer_offset_full) ||
        (state == buffer_offset_half))
    {
        uint32_t rb = get_pcm_sound(&audio_file, buffer, audio_buffer_size / 2);
        if (rb > 0)
            state = buffer_offset_none;
        else
        {
            //ignore
        }
    }
}

void audio_ctl_t::display_time () const
{
    tik_t cur_time;
    tik_t total_time;

    byte_to_time(&cur_time, audio_file.current_position());
    byte_to_time(&total_time, audio_file.size);

    char str[str_time_size];
    snprintf
    (
        str, 
        str_time_size, 
        " %4u:%02u.%03u / %u:%02u.%03u    %c %c ", 
        cur_time.min,
        cur_time.sec,
        cur_time.ms,
        
        total_time.min,
        total_time.sec,
        total_time.ms,
        
        display::print_r_state(repeat_mode),
        display::print_p_state(pause_status)
    );
    display_string_center_c(0, display::offsets::time, str, &font_12, lcd_color_blue, lcd_color_white);
}

ret_code audio_ctl_t::new_song_or_repeat ()
{
    if (!audio_file.is_fake())
    {
        reuse_mad();
    }
    
    if (pause_status == pause_status_t::soft_pause)
    {
        pause_status = pause_status_t::pause;
        BSP_AUDIO_OUT_Pause();
    }
    
    // play song again
    if (repeat_mode)
    {
        ret_code ret;
        if ((ret = audio_file.seek(info.offset)))
        {
            display::error("err seek");
            return ret;
        }
        seeked = 1;
    }
    else //or next song
    {
        ret_code ret;
        if ((ret = viewer.next_song()))
        {
            display::error("err get next song");
            return ret;
        }
        if ((ret = viewer.open_song()))
        {
            viewer.fake_song_and_playlist();
            memset(buff, 0, audio_buffer_size);
            display::error("err open song");
            return ret;
        }
    }
    return 0;
}

ret_code audio_ctl_t::audio_process ()
{
    display_time();
    //end of song reached
    if (audio_file.current_position() >= audio_file.size)
    {
        ret_code ret;
        bool is_fake = audio_file.is_fake();
        if ((ret = new_song_or_repeat()))
            return ret;
        need_redraw |= !is_fake || !audio_file.is_fake();
    }
    next_pcm_part();
    return 0;
}


void BSP_AUDIO_OUT_TransferComplete_CallBack (void)
{
    audio_ctl.state = buffer_offset_full;
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack (void)
{
    audio_ctl.state = buffer_offset_half;
}

void BSP_AUDIO_OUT_Error_CallBack (void)
{
    display::error("err audio out");
}

