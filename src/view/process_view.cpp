#include "view.h"

#include "stm32412g_discovery_audio.h"
#include "display_song.h"
#include "joystick.h"
#include "mp3.h"
#include "display_error.h"
#include <utility>

const uint32_t seek_value = (1024 * 32);

ret_code view::change_volume (int8_t value) noexcept
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
    if (audio_ctl->pause_status == pause_status_t::pause)
        BSP_AUDIO_OUT_Pause();
    display::song_volume(*audio_ctl, state_song_view, (state == state_t::song));
    return 0;
}

ret_code view::inc_volume () noexcept
{
    return change_volume(1);
}

ret_code view::dec_volume () noexcept
{
    return change_volume(-1);
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

ret_code view::seek (uint32_t value, directions::fb::type direction)
{
    uint32_t (* op_in_bound[2]) (uint32_t, uint32_t, uint32_t, uint32_t) = 
        {add_in_bound, sub_in_bound};

    ret_code ret;
    uint32_t new_pos;
    new_pos = audio_ctl->audio_file.current_position();
    new_pos = (op_in_bound[direction])(new_pos, audio_ctl->info.offset, audio_ctl->audio_file.size, value);
    audio_ctl->seeked = 1;
    ret = audio_ctl->audio_file.seek(new_pos);
    if (ret)
        return ret;
    reuse_mad();
    return 0;
}

ret_code view::seek_forward ()
{
    return seek(seek_value, directions::fb::forward);
}

ret_code view::seek_backward ()
{
    return seek(seek_value, directions::fb::backward);
}

ret_code view::change_song (directions::np::type direction)
{
    static uint32_t (playlist::* const do_on_playlist[2]) (playlist const & backup) =
    {
        &playlist::next,
        &playlist::prev
    };
    ret_code ret;
    bool was_fake;
    static playlist backup;
    
    if (next_playlist.value.is_fake() ||
        direction == directions::np::prev)
    {
        was_fake = 1;
        ret = backup.clone(pl.value);
        if (ret)
            return ret;
        ret = (pl.value.*do_on_playlist[direction])(backup);
        if (ret)
            return ret;
    }
    else
    {
        was_fake = 0;
        ret = backup.clone(next_playlist.value);
        if (ret)
            return ret;
        next_playlist.swap(pl);
    }
    reuse_mad();
    ret = open_song_not_found(backup, direction);
    if (ret) 
    {
        if (was_fake)
        {
            pl.value = std::move(backup);
            return ret;
        }
        else
        {
            next_playlist.swap(pl);
            next_playlist.reset();
            return change_song(direction);
        }
    }
    audio_ctl->need_redraw = 1;
    next_playlist.reset();
    return 0;
}

ret_code view::prev_song ()
{
    return change_song(directions::np::prev);
}

ret_code view::next_song ()
{
    return change_song(directions::np::next);
}

ret_code view::set_next_song ()
{
    if (state != state_t::playlist)
        return 0;

    playlist old_pl(std::move(next_playlist.value));
    ret_code ret = plv.value.play(next_playlist.value, old_pl);
    if (ret)
        return ret;
    
    next_playlist.playlist_index = plv.playlist_index;
    audio_ctl->need_redraw = 1;
    return 0;
}

ret_code view::process_next_prev (directions::np::type direction)
{
    static void (pl_list::* const do_on_pl_list[2]) () = 
    {
        &pl_list::next,
        &pl_list::prev
    };
    static uint32_t (playlist_view::* const do_on_playlist_view[2]) () = 
    {
        &playlist_view::next,
        &playlist_view::prev
    };
    
    switch (state)
    {
    case state_t::pl_list:
        (pll.*do_on_pl_list[direction])();
        audio_ctl->need_redraw = 1;
        break;

    case state_t::playlist:
        audio_ctl->need_redraw = 1;
        return (plv.value.*do_on_playlist_view[direction])();
        
    case state_t::song:
        switch (state_song_view)
        {
        case state_song_view_t::volume:
            return change_volume(direction == directions::np::prev? 1 : -1);
            
        case state_song_view_t::seek:
            return seek(seek_value, direction == directions::np::next? directions::fb::backward : directions::fb::forward);

        case state_song_view_t::next_prev:
            return change_song(direction);
        }
    }
    return 0;
}

ret_code view::process_up ()
{
    return process_next_prev(directions::np::prev);
}

ret_code view::process_down ()
{
    return process_next_prev(directions::np::next);
}

ret_code view::play_pause () noexcept
{
    if (audio_ctl->pause_status == pause_status_t::pause)
    {
        BSP_AUDIO_OUT_Resume();
        audio_ctl->pause_status = pause_status_t::play;
    }
    else
    {
        BSP_AUDIO_OUT_Pause();
        audio_ctl->pause_status = pause_status_t::pause;
    }
    audio_ctl->need_redraw = 1;
    return 0;
}

ret_code view::to_end_and_pause () noexcept
{
    switch (audio_ctl->pause_status)
    {
    case pause_status_t::play:
        audio_ctl->pause_status = pause_status_t::soft_pause;
        break;
    case pause_status_t::pause:
        break;
    case pause_status_t::soft_pause:
        audio_ctl->pause_status = pause_status_t::play;
        break;
    };
    audio_ctl->need_redraw = 1;
    return 0;
}

ret_code view::process_left () noexcept
{
    if (state != state_t::pl_list)
    {
        state = prev(state);
        audio_ctl->need_redraw = 1;
    }
    return 0;
}

ret_code view::play_new_playlist ()
{
    ret_code ret;
    playlist old_pl(std::move(pl.value));

    ret = plv.value.play(pl.value, old_pl);
    if (ret)
        return ret;
    reuse_mad();
    ret = open_song_not_found(old_pl);
    if (ret)
    {
        pl.value = std::move(old_pl);
        return ret;
    }

    pl.playlist_index = plv.playlist_index;
    next_playlist.reset();
    return 0;
}

ret_code view::process_right ()
{
    ret_code ret;
    switch (state)
    {
    case state_t::pl_list:
        state = state_t::playlist;
        ret = pll.open_selected(plv.value, plv.playlist_index);
        if (ret)
            return ret;
        audio_ctl->need_redraw = 1;
        break;

    case state_t::playlist:
        if ((ret = play_new_playlist()))
            return ret;
        audio_ctl->need_redraw = 1;
        break;
        
    case state_t::song:
        state_song_view = roll(state_song_view);
        display::song_volume(*audio_ctl, state_song_view, 1);
        break;
    }
    return 0;
}

ret_code view::toggle_repeat () noexcept
{
    audio_ctl->repeat_mode ^= 1;
    display::song_volume(*audio_ctl, state_song_view, (state == state_t::song));
    return 0;
}

ret_code view::process_center ()
{
    switch (state)
    {
    case state_t::pl_list:
        if (pll.check_near(pl.playlist_index))
        {
            ret_code ret = pll.open_index(plv.value, pl.playlist_index, plv.playlist_index);
            if (ret)
                return ret;
            state = state_t::playlist;
        }
        else
        {
            if (pl.playlist_index != pl_list::max_plb_files)
                pll.seek(pl.playlist_index);
        }
        audio_ctl->need_redraw = 1;
        break;
        
    case state_t::playlist:
        if (plv.value.check_near(pl.value))
        {
            state = state_t::song;
            audio_ctl->need_redraw = 1;
        }
        else
        {
            if (pl.value.lpl.header.cnt_songs != 0)
                return to_playing_pos(pl.value.lpl);
        }
        break;
        
    case state_t::song:
        toggle_repeat();
        break;
    }
    return 0;
}

ret_code view::new_song_or_repeat ()
{
    if (!audio_ctl->audio_file.is_fake())
    {
        reuse_mad();
    }

    if (audio_ctl->pause_status == pause_status_t::soft_pause)
    {
        audio_ctl->pause_status = pause_status_t::pause;
        BSP_AUDIO_OUT_Pause();
    }

    // play song again
    audio_ctl->seeked = 1;
    if (audio_ctl->repeat_mode)
    {
        ret_code ret;
        if ((ret = audio_ctl->audio_file.seek(audio_ctl->info.offset)))
        {
            display::error("err seek");
            return ret;
        }
    }
    else //or next song
    {
        ret_code ret;
        if ((ret = next_song()))
        {
            display::error("err get next song");
            memset(audio_ctl->buff, 0, audio_ctl->audio_buffer_size);
            return ret;
        }
    }
    return 0;
}

ret_code view::jmp(uint32_t pos)
{
    switch (state)
    {
    case state_t::pl_list:
        audio_ctl->need_redraw = 1;
        pll.seek(pos);
        return 0;
        
    case state_t::playlist:
        audio_ctl->need_redraw = 1;
        return plv.value.seek(pos);
        
    case state_t::song:
        return 0;
    }
    return 0;
}

