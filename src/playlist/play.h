#ifndef PLAY_H
#define PLAY_H

#include "playlist_structures.h"
#include "playlist.h"
#include "light_playlist.h"
#include "playlist_view.h"

/*
//init
file_descriptor fd_plv;
playlist_view plv;
open(&fd_plv, path, path_len);
init_playlist_view(&plv, &fd_plv);

file_descriptor fd_pl;
playlist pl;
open(&fd_pl, path, path_len);
init_playlist(&pl, &fd_pl);


//playlist_view
    //up / down
    up(&plv);
    down(&plv);

    //play
    play(&plv, &pl);

    //change playlist
    open_playlist(&plv, path, path_len);
    
    //print playlist_view
    char song_name[view_cnt][song_name_sz + 1];
    char group_name[view_cnt][group_name_sz + 1];
    char selected[view_cnt];
        // 0
        // 1 - selected
        // 2 - playing
        // 3 - playing and selected
    char number[view_cnt][3 + 1];
    
    print_playlist_view(&plv, &pl_p, song_name, group_name, selected, number);
    
    for (size_t i = 0; i != view_cnt; ++i)
    {
        if (selected[i] == 3)
        {
            printf("\033[01;33m");
            printf("%s ", number[i]);
            printf("%s\n", group_name[i]);
            printf("\033[01;32m");
            printf("%s\n", song_name[i]);
            continue;
        }
        
        if (selected[i] == 1)
        {
            printf("\033[01;32m");
        }
        else if (selected[i] == 2)
        {
            printf("\033[01;33m");
        }
        else
        {
            printf("\033[00;00m");
        }
        printf("%s ", number[i]);
        printf("%s\n", group_name[i]);
        printf("%s\n", song_name[i]);
        printf("\033[00;00m");
    }

//playlist
    //end of song reached
    next_playlist(&pl);
    open_song(&pl, &fd_song);
*/

#endif

