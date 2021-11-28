#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>
#include "playlist.h"
#include "playlist_view.h"
#include "pl_list.h"
#include "audio.h"
#include "view_states.h"
#include "direction_t.h"
#include "find_song.h"

struct view
{
    consteval view (audio_ctl_t * _audio_ctl) :
        plv(),
        pl(),
        next_playlist(),
        state(state_t::pl_list),
        old_state(state),
        state_song_view(state_song_view_t::volume),
        audio_ctl(_audio_ctl),
        pll(),
        finder()
    {}
    
    constexpr void reset ()
    {
        pl.reset();
        plv.reset();
        next_playlist.reset();
        state = state_t::pl_list;
        old_state = state;
        state_song_view = state_song_view_t::volume;
        pll.reset();
        finder.reset();
    }

    ~view () = default;
    
    view (view const &) = delete;
    view & operator = (view const &) = delete;
    view (view &&) = delete;
    view & operator = (view &&) = delete;

    ret_code init (filename_t * path, uint32_t len);
    void display ();
    void fake_song_and_playlist ();
    ret_code new_song_or_repeat ();

    ret_code process_up         ();
    ret_code process_down       ();
    ret_code process_next_prev  (directions::np::type direction);
    ret_code process_left       () noexcept;
    ret_code process_right      ();
    ret_code process_center     ();

    ret_code find               (find_pattern const & pattern);
    ret_code find_next          ();
    ret_code play_pause         () noexcept;
    ret_code to_end_and_pause   () noexcept;
    ret_code toggle_repeat      () noexcept;
    ret_code inc_volume         () noexcept;
    ret_code dec_volume         () noexcept;
    ret_code seek_forward       ();
    ret_code seek_backward      ();
    ret_code prev_song          ();
    ret_code next_song          ();
    ret_code set_next_song      ();
    ret_code jmp                (uint32_t pos);

    ret_code do_nothing         () noexcept; // can use in table of function
    ret_code send_info          () noexcept;
    

    struct viewing
    {
        uint32_t playlist_index;
        playlist_view value;
        
        constexpr viewing () :
            playlist_index(pl_list::max_plb_files),
            value()
        {}

        constexpr void reset ()
        {
            playlist_index = pl_list::max_plb_files;
            value.reset();
        }
    };

    struct playing
    {
        uint32_t playlist_index;
        playlist value;

        constexpr playing () :
            playlist_index(pl_list::max_plb_files),
            value()
        {}
            
        constexpr void reset ()
        {
            playlist_index = pl_list::max_plb_files;
            value.make_fake();
        }

        constexpr void swap (playing & other)
        {
            value.swap(other.value);
            std::swap(playlist_index, other.playlist_index);
        }
    };
    
    viewing plv;
    playing pl;
    playing next_playlist;
    
    state_t state;
    state_t old_state;
    state_song_view_t state_song_view;

    audio_ctl_t * audio_ctl;
    pl_list pll;
    find_song finder;
    
    
private:
    ret_code change_volume (int8_t value) noexcept;
    ret_code seek (uint32_t value, directions::fb::type direction /* CHANGE 0 - backward, 1 - forward */);
    ret_code change_song (directions::np::type direction);
    ret_code play_new_playlist ();
    ret_code to_playing_pos (light_playlist const & lpl);
    ret_code find_common (light_playlist const & backup);

    ret_code open_song_impl ();
    ret_code open_song_not_found (playlist const & backup,  directions::np::type direction = directions::np::next);
};


extern view viewer;

#endif

