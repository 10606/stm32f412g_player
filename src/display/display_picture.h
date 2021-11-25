#ifndef DISPLAY_PICTURE_H
#define DISPLAY_PICTURE_H

#include <stdint.h>
#include <stddef.h>
#include <type_traits>

namespace display
{

struct picture_info_t 
{
    picture_info_t () :
        song_picture_number(0),
        start_picture_number(0)
    {}
    
    void update_song_pic_num ()
    {
        song_picture_number = (song_picture_number + 1) % std::extent_v <decltype(song)>;
    }
    
    void update_start_pic_num ()
    {
        start_picture_number = (start_picture_number + 1) % std::extent_v <decltype(err)>;
    }
    
    const void * song_pic ()
    {
        return song[song_picture_number];
    }
    
    const void * start_pic ()
    {
        return err[start_picture_number];
    }

    uint16_t color; // at 208, 6
    
private:
    uint8_t song_picture_number;
    uint8_t start_picture_number;
    
    static const void * const song[7];
    static const void * const err[2];
};

extern picture_info_t picture_info;


void start_image ();
void song_image ();

}

#endif

