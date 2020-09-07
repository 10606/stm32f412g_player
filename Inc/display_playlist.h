#ifndef DISPLAY_PLAYLIST_H
#define DISPLAY_PLAYLIST_H

#include "play.h"
#include "main.h"
#include "FAT.h"
#include "stm32412g_discovery_audio.h"
#include <stdint.h>

void display_playlist (playlist_view * plv, playlist * pl_p);

#endif

