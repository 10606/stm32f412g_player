#include "view.h"

#include "display_song.h"
#include "display_playlists.h"
#include "usb_send.h"
#include <new>

ret_code view::init (filename_t * path, uint32_t len)
{
    ret_code ret;
    ret = pll.init(path, len);
    if (ret)
        return ret;
    return 0;
}

void view::display ()
{
    sender.send_state(state);
    display::cur_song(*audio_ctl, pl.value, repeat_counter);
    display_playlist.cur_playlist(plv.value, state, old_state);
    display_playlist.cur_pl_list(pll, state, old_state);
    display::song(*audio_ctl, state_song_view, repeat_counter, state, old_state);
    sender.flush();
    old_state = state;
}

ret_code view::open_song_impl ()
{
    if (pl.value.lpl.header.cnt_songs == 0)
    {
        audio_ctl->audio_file.init_fake();
        audio_ctl->info.offset = 0;
        return 0;
    }

    ret_code ret;
    file_descriptor old_audio_file = audio_ctl->audio_file;
    mp3_info old_info = audio_ctl->info;
    if ((ret = open(&FAT_info, &audio_ctl->audio_file, pl.value.path, pl.value.lpl.song.path_len)))
        return ret;
    audio_ctl->seeked = 1;
    get_length(&audio_ctl->audio_file, &audio_ctl->info);
    if ((ret = audio_ctl->audio_file.seek(audio_ctl->info.offset)))
    {
        audio_ctl->audio_file = old_audio_file;
        audio_ctl->info = old_info;
        return ret;
    }
    return 0;
}

ret_code view::open_song_not_found (playlist const & backup, directions::np::type direction)
{
    static uint32_t (playlist::* const np_playlist[2]) (playlist const & backup) = 
    {
        &playlist::next,
        &playlist::prev
    };
    
    ret_code ret_song = 0;
    ret_code ret_playlist = 0;
    for (uint32_t i = 0; i != pl.value.lpl.header.cnt_songs; ++i)
    {
        if (!(ret_song = open_song_impl()))
            return 0;
        
        if ((ret_playlist = (pl.value.*np_playlist[direction])(backup)))
            return ret_playlist;
    }
    return ret_song;
}

void view::fake_song_and_playlist ()
{
    pl.value.make_fake();
    audio_ctl->audio_file.init_fake();
}

ret_code view::do_nothing () noexcept
{
    return 0;
}

ret_code view::send_info () noexcept
{
    audio_ctl->need_redraw = 1;
    return 0;
}

ret_code view::find (find_pattern const & pattern)
{
    light_playlist playing = plv.value.lpl_with_wrong_pos();
    ret_code ret;
    ret = playing.seek(plv.value.get_pos());
    if (ret)
        return ret;
    finder = find_song(playing, pattern);
    return find_common(playing);
}

ret_code view::find_next ()
{
    if (!plv.value.compare(finder.playlist))
        return find(finder.search_pattern());
    
    light_playlist playing = finder.playlist;
    ret_code ret;
    ret = finder.playlist.seek(plv.value.get_pos());
    if (ret)
        return ret;
    return find_common(playing);
}

ret_code view::find_common (light_playlist const & backup)
{
    ret_code ret;
    ret = finder.next(backup.fd);
    if (ret)
        return (ret == find_song::not_found)? 0 : ret;
    ret = to_playing_pos(finder.playlist);
    if (ret)
    {
        finder.playlist = backup;
        return ret;
    }
    audio_ctl->need_redraw = 1;
    return 0;
}

ret_code view::to_playing_pos (light_playlist const & lpl)
{
    if (!plv.value.compare(lpl))
    {
        ret_code ret;
        ret = plv.value.to_playing_playlist(lpl);
        if (ret)
            return ret;
        pll.seek(pl.playlist_index);
        plv.playlist_index = pl.playlist_index;
    }
    audio_ctl->need_redraw = 1;
    return plv.value.seek(lpl.pos);
}

view viewer(&audio_ctl);

