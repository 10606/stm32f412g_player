#ifndef DISPLAY_PLAYLISTS_H
#define DISPLAY_PLAYLISTS_H

#include "playlist_view.h"
#include "playlist.h"
#include "pl_list.h"
#include <stdint.h>

namespace display
{

void cur_pl_list  (pl_list & pll,       uint32_t playing_pl, bool to_screen, bool redraw_screen, bool & need_redraw);
void cur_playlist (playlist_view & plv, playlist const & pl, bool to_screen, bool redraw_screen, bool & need_redraw);

}

#endif

