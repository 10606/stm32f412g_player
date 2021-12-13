#ifndef DISPLAY_PICTURE_H
#define DISPLAY_PICTURE_H

#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <utility>

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
    
    static const void * const song[9];
    static const void * const err[2];
};

extern picture_info_t picture_info;

struct write_region_t
{
    write_region_t 
    (
        std::pair <uint16_t, uint16_t> const _pos,
        std::pair <uint16_t, uint16_t> const _size,
        uint32_t const _p_size,
        bool const _need_audio
    ) :
        pos(_pos),
        size(_size),
        p_size(_p_size),
        need_audio(_need_audio),
        
        line_cnt(0),
        p_old_size(0)
    {}
    
    // i don't want to inline it
    __attribute__((noinline))
    bool operator () (uint16_t const * line);
    
private:
    std::pair <uint16_t, uint16_t> const pos;
    std::pair <uint16_t, uint16_t> const size;
    uint32_t const p_size;
    bool const need_audio;
    
    uint32_t line_cnt;
    uint32_t p_old_size;
};

void display_picture
(
    void const * addr,
    std::pair <uint16_t, uint16_t> const pos,
    std::pair <uint16_t, uint16_t> const size,
    bool const need_audio
);

void start_image ();
void song_image ();

}

#endif

