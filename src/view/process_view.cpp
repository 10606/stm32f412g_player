#include "view.h"

#include "stm32412g_discovery_audio.h"
#include "display.h"
#include "joystick.h"
#include "mp3.h"
#include <utility>

const uint32_t seek_value = (1024 * 32);

uint32_t view::change_volume (bool & need_redraw, int8_t value)
{
    if (value > 100)
        value = 100;
    if (value < -100)
        value = -100;
    if (audio_ctl->volume + value > 100)
        audio_ctl->volume = 100;
    else if (audio_ctl->volume < -value)
        audio_ctl->volume = 0;
    else
        audio_ctl->volume += value;
    BSP_AUDIO_OUT_SetVolume(audio_ctl->volume);
    display::song_volume(*audio_ctl, state_song_view, (state == state_t::song), need_redraw);
    return 0;
}

uint32_t view::inc_volume (bool & need_redraw)
{
    return change_volume(need_redraw, 1);
}

uint32_t view::dec_volume (bool & need_redraw)
{
    return change_volume(need_redraw, -1);
}

inline uint32_t add_in_bound (uint32_t value, uint32_t a, uint32_t b, uint32_t add)
{
    if (value + add < value)
        return b;
    if (value + add > b)
        return b;
    return value + add;
}

inline uint32_t sub_in_bound (uint32_t value, uint32_t a, uint32_t b, uint32_t add)
{
    if (value < add)
        return a;
    if (value - add < a)
        return a;
    return value - add;
}

uint32_t view::seek (bool & need_redraw, uint32_t value, uint8_t direction /* 0 - backward, 1 - forward */)
{
    uint32_t ret, new_pos;
    new_pos = audio_ctl->audio_file.current_position();
    new_pos = direction? 
        add_in_bound(new_pos, 0, audio_ctl->audio_file.size, value) :
        sub_in_bound(new_pos, 0, audio_ctl->audio_file.size, value);
    audio_ctl->seeked = 1;
    ret = audio_ctl->audio_file.seek(new_pos);
    if (ret)
        return ret;
    return 0;
}

uint32_t view::seek_forward (bool & need_redraw)
{
    return seek(need_redraw, seek_value, 1);
}

uint32_t view::seek_backward (bool & need_redraw)
{
    return seek(need_redraw, seek_value, 0);
}

uint32_t view::change_song (bool & need_redraw, uint8_t direction /* 0 - next, 1 - prev */)
{
    static uint32_t (playlist::* const do_on_playlist[2]) () =
    {
        &playlist::next,
        &playlist::prev
    };
    uint32_t ret;
    ret = (pl.*do_on_playlist[direction])();
    if (ret)
        return ret;
    else
    {
        reuse_mad();
        if ((ret = open_song_not_found(direction))) 
            return ret;
    }
    need_redraw = 1;
    return 0;
}

uint32_t view::prev_song (bool & need_redraw)
{
    return change_song(need_redraw, 1);
}

uint32_t view::next_song (bool & need_redraw)
{
    return change_song(need_redraw, 0);
}

uint32_t view::process_up_down (bool & need_redraw, uint8_t direction /* 0 - down, 1 - up */)
{
    static void (pl_list::* const do_on_pl_list[2]) () = 
    {
        &pl_list::down,
        &pl_list::up
    };
    static uint32_t (playlist_view::* const do_on_playlist_view[2]) () = 
    {
        &playlist_view::down,
        &playlist_view::up
    };
    
    switch (state)
    {
    case state_t::pl_list:
        (pll.*do_on_pl_list[direction])();
        need_redraw = 1;
        break;

    case state_t::playlist:
        need_redraw = 1;
        return (plv.*do_on_playlist_view[direction])();
        
    case state_t::song:
        switch (state_song_view)
        {
        case state_song_view_t::volume:
            return change_volume(need_redraw, direction? 1 : -1);
            
        case state_song_view_t::seek:
            return seek(need_redraw, seek_value, direction);

        case state_song_view_t::next_prev:
            return change_song(need_redraw, direction);
        }
    }
    return 0;
}

uint32_t view::process_up (bool & need_redraw)
{
    return process_up_down(need_redraw, 1);
}

uint32_t view::process_down (bool & need_redraw)
{
    return process_up_down(need_redraw, 0);
}

uint32_t view::play_pause (bool & need_redraw)
{
    if (audio_ctl->pause_status == 1)
    {
        BSP_AUDIO_OUT_Resume();
        audio_ctl->pause_status = 0;
    }
    else
    {
        BSP_AUDIO_OUT_Pause();
        audio_ctl->pause_status = 1;
    }
    need_redraw = 1;
    return 0;
}


uint32_t view::process_left (bool & need_redraw)
{
    if (state != state_t::pl_list)
    {
        state = prev(state);
        need_redraw = 1;
    }
    return 0;
}

uint32_t view::play_new_playlist ()
{
    uint32_t ret;
    playlist old_pl(std::move(pl));

    ret = plv.play(pl);
    if (ret)
        return ret;
    ret = open_song_not_found(0);
    if (ret)
    {
        pl = std::move(old_pl);
        return ret;
    }

    playing_playlist = selected_playlist;
    reuse_mad();
    return 0;
}

uint32_t view::process_right (bool & need_redraw)
{
    uint32_t ret;
    switch (state)
    {
    case state_t::pl_list:
        state = state_t::playlist;
        ret = pll.open_selected(plv, selected_playlist);
        if (ret)
            return ret;
        need_redraw = 1;
        break;

    case state_t::playlist:
        if ((ret = play_new_playlist()))
            return ret;
        need_redraw = 1;
        break;
        
    case state_t::song:
        state_song_view = roll(state_song_view);
        display::song_volume(*audio_ctl, state_song_view, 1, need_redraw);
        break;
    }
    return 0;
}

uint32_t view::toggle_repeat (bool & need_redraw)
{
    audio_ctl->repeat_mode ^= 1;
    display::song_volume(*audio_ctl, state_song_view, 1, need_redraw);
    return 0;
}

uint32_t view::process_center (bool & need_redraw)
{
    switch (state)
    {
    case state_t::pl_list:
        if (pll.check_near(playing_playlist))
        {
            uint32_t ret = pll.open_index(plv, playing_playlist, selected_playlist);
            if (ret)
                return ret;
            state = state_t::playlist;
        }
        else
        {
            if (playing_playlist != max_plb_files)
                pll.seek(playing_playlist);
        }
        need_redraw = 1;
        break;
        
    case state_t::playlist:
        if (plv.check_near(pl))
        {
            state = state_t::song;
            need_redraw = 1;
        }
        else
        {
            if (pl.header.cnt_songs != 0)
            {
                uint32_t ret;
                if (!plv.compare(pl))
                {
                    ret = plv.to_playing_playlist(pl);
                    if (ret)
                        return ret;
                    pll.seek(playing_playlist);
                    selected_playlist = playing_playlist;
                }
                need_redraw = 1;
                ret = plv.seek(pl.pos);
                if (ret)
                    return ret;
            }
        }
        break;
        
    case state_t::song:
        toggle_repeat(need_redraw);
        break;
    }
    return 0;
}

