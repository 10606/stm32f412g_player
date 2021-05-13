#ifndef DISPLAY_PICTURE_H
#define DISPLAY_PICTURE_H

#include <stdint.h>
#include <stddef.h>

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
        song_picture_number = (song_picture_number + 1) % cnt_pictures_song;
    }
    
    void update_start_pic_num ()
    {
        start_picture_number = (start_picture_number + 1) % cnt_pictures_start;
    }
    
    const uint16_t * song_pic ()
    {
        return song + song_picture_number * offset;
    }
    
    const uint16_t * start_pic ()
    {
        return err + start_picture_number * offset;
    }

private:
    uint8_t song_picture_number;
    uint8_t start_picture_number;
    
    static const uint16_t * const song;
    static const uint16_t * const err;
    
    static const size_t offset = 0x10000; // in uint16_t
    static const uint8_t cnt_pictures_song = 2;
    static const uint8_t cnt_pictures_start = 2;

};

extern picture_info_t picture_info;


void start_image ();
void song_image (bool & need_redraw);

}

#endif

