#include "view.h"

#include "display.h"
#include "usb_send.h"
#include <new>

view::view (audio_ctl_t * _audio_ctl)
{
    audio_ctl = _audio_ctl;
    state = state_t::pl_list;
    old_state = state;
    state_song_view = state_song_view_t::volume;
    playing_playlist = max_plb_files;
    selected_playlist = max_plb_files;
}

uint32_t view::init (char (* path)[12], uint32_t len)
{
    uint32_t ret;
    ret = init_pl_list(&pll, path, len);
    if (ret)
        return ret;

    ret = plv.init();
    if (ret)
        return ret;
    
    ret = plv.play(&pl);
    if (ret)
        return ret;
    playing_playlist = selected_playlist;
    return 0;
}

view::~view ()
{
    destroy_pl_list(&pll);
}

void view::display (uint8_t * need_redraw)
{
    char ts_playlist = (state == state_t::playlist);
    char ts_pl_list = (state == state_t::pl_list);
    char ts_song = (state == state_t::song);

    send_state(state);
    display_playlist(&plv, &pl, ts_playlist, need_redraw);
    display_pl_list(&pll, playing_playlist, &pl, ts_pl_list, need_redraw);
    display_song(&pl, audio_ctl, state_song_view, ts_song, (old_state != state_t::song), need_redraw);
    old_state = state;
}

uint32_t view::open_song ()
{
    if (pl.header.cnt_songs == 0)
    {
        init_fake_file_descriptor(&audio_ctl->audio_file);
        audio_ctl->info.offset = 0;
        return 0;
    }

    uint32_t ret;
    if ((ret = open(&FAT_info, &audio_ctl->audio_file, pl.path, pl.song.path_len)))
        return ret;
    audio_ctl->seeked = 1;
    get_length(&audio_ctl->audio_file, &audio_ctl->info);
    if ((ret = f_seek(&audio_ctl->audio_file, audio_ctl->info.offset)))
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
    for (uint32_t i = 0; i != pl.header.cnt_songs; ++i)
    {
        if ((ret = open_song()))
        {
            if (ret != not_found)
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
    init_fake_file_descriptor(&audio_ctl->audio_file);
}

view_holder viewer;

