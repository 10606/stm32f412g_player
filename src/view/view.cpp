#include "view.h"

#include "display_song.h"
#include "display_playlists.h"
#include "usb_send.h"
#include <new>

uint32_t view::init (filename_t * path, uint32_t len)
{
    uint32_t ret;
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

uint32_t view::open_song_not_found (directions::np::type direction)
{
    uint32_t (playlist::* np_playlist[2]) () = 
    {
        &playlist::next,
        &playlist::prev
    };
    
    uint32_t ret = 0;
    for (uint32_t i = 0; i != pl.lpl.header.cnt_songs; ++i)
    {
        if (!(ret = open_song()))
            return 0;
        
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

uint32_t view::do_nothing ()
{
    return 0;
}

uint32_t view::send_info ()
{
    audio_ctl->need_redraw = 1;
    return 0;
}

uint32_t view::find (find_pattern const & pattern)
{
    light_playlist playing = plv.lpl_with_wrong_pos();
    uint32_t ret;
    ret = playing.seek(plv.get_pos());
    if (ret)
        return ret;
    finder = find_song(playing, pattern);
    return find_common(playing);
}

uint32_t view::find_next ()
{
    if (!plv.compare(finder.playlist))
        return find(finder.search_pattern());
    
    light_playlist playing = finder.playlist;
    uint32_t ret;
    ret = finder.playlist.seek(plv.get_pos());
    if (ret)
        return ret;
    return find_common(playing);
}

uint32_t view::find_common (light_playlist const & backup)
{
    uint32_t ret;
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

uint32_t view::to_playing_pos (light_playlist const & lpl)
{
    uint32_t ret;

    if (!plv.compare(lpl))
    {
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

