#ifndef VIEW_STATES_H
#define VIEW_STATES_H

#include <stddef.h>

enum class state_song_view_t
{
    volume,
    seek,
    next_prev
};

enum class state_t
{
    pl_list,
    playlist,
    song
};

inline state_t prev (state_t const & value)
{
    switch (value)
    {
    case state_t::pl_list:
    case state_t::playlist:
    default:
        return state_t::pl_list;
    case state_t::song:
        return state_t::playlist;
    }
}

inline state_t next (state_t const & value)
{
    switch (value)
    {
    case state_t::pl_list:
    default:
        return state_t::playlist;
    case state_t::playlist:
    case state_t::song:
        return state_t::song;
    }
}

inline state_song_view_t roll (state_song_view_t const & value)
{
    switch (value)
    {
    case state_song_view_t::volume:
        return state_song_view_t::seek;
    case state_song_view_t::seek:
        return state_song_view_t::next_prev;
    case state_song_view_t::next_prev:
    default:
        return state_song_view_t::volume;
    }
}

#endif

