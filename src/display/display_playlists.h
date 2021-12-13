#ifndef DISPLAY_PLAYLISTS_H
#define DISPLAY_PLAYLISTS_H

#include "playlist_view.h"
#include "playlist.h"
#include "pl_list.h"
#include "view_states.h"
#include <stdint.h>

namespace display
{

template <typename ... T>
struct playlists_variadic
{
    constexpr playlists_variadic (T ... values) :
        playlists_value{&values.value ...},
        playlists_index{&values.playlist_index ...}
    {}

    void cur_pl_list  (pl_list & pll,       state_t cur_state, state_t old_state);
    void cur_playlist (playlist_view & plv, state_t cur_state, state_t old_state);
    
    std::array <playlist const *, sizeof ... (T)> playlists_value;
    std::array <uint32_t const *, sizeof ... (T)> playlists_index;
};

template <typename ... T>
struct type_sequence;

template <typename T, size_t count, typename ... V>
struct make_type_sequence : make_type_sequence <T, count - 1, T, V ...>
{};

template <typename T, typename ... V>
struct make_type_sequence <T, 0, V ...>
{
    typedef type_sequence <V ...> type;
};

template <typename T>
struct playlists_def;

template <typename ... T>
struct playlists_def <type_sequence <T ...> >
{
    typedef playlists_variadic <T ...> type;
};

template <size_t count>
using playlists = 
    typename playlists_def 
    <
        typename make_type_sequence 
        <
            playing const &, count
        > :: type
    > :: type;

}

#endif

