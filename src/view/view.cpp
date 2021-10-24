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
    display::cur_song(*audio_ctl, pl);
    display::cur_playlist(plv, pl, state, old_state);
    display::cur_pl_list(pll, playing_playlist, state, old_state);
    display::song(*audio_ctl, state_song_view, state, old_state);
    sender.flush();
    old_state = state;
}

ret_code view::open_song_impl ()
{
    if (pl.lpl.header.cnt_songs == 0)
    {
        audio_ctl->audio_file.init_fake();
        audio_ctl->info.offset = 0;
        return 0;
    }

    ret_code ret;
    file_descriptor old_audio_file = audio_ctl->audio_file;
    if ((ret = open(&FAT_info, &audio_ctl->audio_file, pl.path, pl.lpl.song.path_len)))
        return ret;
    audio_ctl->seeked = 1;
    get_length(&audio_ctl->audio_file, &audio_ctl->info);
    if ((ret = audio_ctl->audio_file.seek(audio_ctl->info.offset)))
    {
        audio_ctl->audio_file = old_audio_file;
        return ret;
    }
    return 0;
}

ret_code view::open_song_not_found (playlist const & backup, directions::np::type direction)
{
    static uint32_t (playlist::* const np_playlist[2]) (light_playlist const & backup) = 
    {
        &playlist::next,
        &playlist::prev
    };
    
    ret_code ret_song = 0;
    ret_code ret_playlist = 0;
    for (uint32_t i = 0; i != pl.lpl.header.cnt_songs; ++i)
    {
        if (!(ret_song = open_song_impl()))
            return 0;
        
        if ((ret_playlist = (pl.*np_playlist[direction])(backup.lpl)))
            return ret_playlist;
    }
    return ret_song;
}

ret_code view::open_song ()
{
    playlist backup;
    ret_code ret = backup.clone(pl);
    if (ret)
        return ret;
    ret = open_song_not_found(backup);
    if (ret)
    {
        pl = std::move(backup);
        return ret;
    }
    return 0;
}

void view::fake_song_and_playlist ()
{
    pl.make_fake();
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
    light_playlist playing = plv.lpl_with_wrong_pos();
    ret_code ret;
    ret = playing.seek(plv.get_pos());
    if (ret)
        return ret;
    finder = find_song(playing, pattern);
    return find_common(playing);
}

ret_code view::find_next ()
{
    if (!plv.compare(finder.playlist))
        return find(finder.search_pattern());
    
    light_playlist playing = finder.playlist;
    ret_code ret;
    ret = finder.playlist.seek(plv.get_pos());
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
    if (!plv.compare(lpl))
    {
        ret_code ret;
        ret = plv.to_playing_playlist(lpl);
        if (ret)
            return ret;
        pll.seek(playing_playlist);
        selected_playlist = playing_playlist;
    }
    audio_ctl->need_redraw = 1;
    return plv.seek(lpl.pos);
}

view viewer(&audio_ctl);

