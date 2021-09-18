#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>
#include "playlist.h"
#include "playlist_view.h"
#include "pl_list.h"
#include "audio.h"
#include "view_states.h"
#include "direction_t.h"
#include <optional>

struct view
{
    view (audio_ctl_t *);
    ~view () = default;
    
    view (view const &) = delete;
    view & operator = (view const &) = delete;
    view (view &&) = delete;
    view & operator = (view &&) = delete;

    uint32_t init (filename_t * path, uint32_t len);
    void display ();
    uint32_t open_song ();
    uint32_t open_song_not_found (directions::np::type direction = directions::np::next);
    void fake_song_and_playlist ();

    uint32_t process_up        ();
    uint32_t process_down      ();
    uint32_t process_next_prev (directions::np::type direction);
    uint32_t process_left      ();
    uint32_t process_right     ();
    uint32_t process_center    ();

    uint32_t play_pause        ();
    uint32_t to_end_and_pause  ();
    uint32_t toggle_repeat     ();
    uint32_t inc_volume        ();
    uint32_t dec_volume        ();
    uint32_t seek_forward      ();
    uint32_t seek_backward     ();
    uint32_t prev_song         ();
    uint32_t next_song         ();

    uint32_t do_nothing        (); // can use in table of function
    uint32_t send_info         ();

    
    uint32_t playing_playlist;
    uint32_t selected_playlist;
    state_t state;
    state_t old_state;
    state_song_view_t state_song_view;

    audio_ctl_t * audio_ctl;
    pl_list pll;
    playlist pl;
    playlist_view plv;
    
    
private:
    uint32_t change_volume (int8_t value);
    uint32_t seek (uint32_t value, directions::fb::type direction /* CHANGE 0 - backward, 1 - forward */);
    uint32_t change_song (directions::np::type direction);
    uint32_t play_new_playlist ();
};


struct view_holder
{
    void reset ()
    {
        value.reset();
    }
    
    uint32_t init (filename_t * path, uint32_t len, audio_ctl_t * audio_ctl)
    {
        value.emplace(audio_ctl);
        return value.value().init(path, len);
    }
    
    view & operator () ()
    {
        return value.value();
    }

private:
    std::optional <view> value;
};

extern view_holder viewer;

#endif

