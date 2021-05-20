#include "view.h"

#include "display_song.h"
#include "display_playlists.h"
#include "usb_send.h"
#include <new>

view::view (audio_ctl_t * _audio_ctl)
{
    audio_ctl = _audio_ctl;
    state = state_t::pl_list;
    old_state = state;
    state_song_view = state_song_view_t::volume;
    playing_playlist = pl_list::max_plb_files;
    selected_playlist = pl_list::max_plb_files;
}

uint32_t view::init (char (* path)[12], uint32_t len)
{
    uint32_t ret;
    ret = pll.init(path, len);
    if (ret)
        return ret;
    return 0;
}

void view::display (bool & need_redraw)
{
    send_state(state);
    display::cur_song(pl, need_redraw);
    display::cur_playlist(plv, pl, state, old_state, need_redraw);
    display::cur_pl_list(pll, playing_playlist, state, old_state, need_redraw);
    display::song(*audio_ctl, state_song_view, state, old_state, need_redraw);
    old_state = state;
}

uint32_t view::open_song ()
{
    if (pl.lpl.header.cnt_songs == 0)
    {
        audio_ctl->audio_file.init_fake();
        audio_ctl->info.offset = 0;
        return 0;
    }

    uint32_t ret;
    if ((ret = open(&FAT_info, &audio_ctl->audio_file, pl.path, pl.lpl.song.path_len)))
        return ret;
    audio_ctl->seeked = 1;
    get_length(&audio_ctl->audio_file, &audio_ctl->info);
    if ((ret = audio_ctl->audio_file.seek(audio_ctl->info.offset)))
        return ret;
    return 0;
}

uint32_t view::open_song_not_found (uint8_t direction)
{
    uint32_t (playlist::* np_playlist[2]) () = 
    {
        &playlist::next,
        &playlist::prev
    };
    
    uint32_t ret = 0;
    for (uint32_t i = 0; i != pl.lpl.header.cnt_songs; ++i)
    {
        if ((ret = open_song()))
        {
            if (ret != err::not_found)
                return ret;
        }
        else
        {
            return 0;
        }
        
        if ((ret = (pl.*np_playlist[direction])()))
            return ret;
    }
    return ret;
}

void view::fake_song_and_playlist ()
{
    pl.make_fake();
    audio_ctl->audio_file.init_fake();
}

uint32_t view::do_nothing (bool & need_redraw)
{
    return 0;
}

uint32_t view::send_info (bool & need_redraw)
{
    need_redraw = 1;
    return 0;
}

view_holder viewer;

