#ifndef ID3_H
#define ID3_H

#include "FAT.h"
#include <stdint.h>


struct id3_v1_header
{
    char header[3]; //TAG
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    char genre;
};


struct id3_v2_header
{
    char marker[3]; //ID3
    char version;
    char sub_version;
    char flags;
    char length[4]; //pidoracy
};


struct mp3_header
{
    /*
    uint32_t
        marker : 11, //0b11111111111 (AND with 0xffe0)
        version : 2, //MPEG-1 = 0b11
        layer : 2,
        has_CRC : 1,
        
        
        bitrate : 4,
        freq : 2,
        offset : 1,
        private_bit : 1,
        
        channel_mode : 2,
        mode_ext : 2,
        copyrate : 1,
        original : 1,
        emphasis : 2;
    */
    unsigned char value[4];
};


struct xing_header
{
    char header[4];
    char _[4];
    unsigned char length[4];
};


struct vbri_header
{
    char header[4];
    char _[6];
    uint16_t length_l; 
    uint16_t length_h; 
};


struct mp3_info
{
    uint32_t offset;
    float length;
};


void get_length (file_descriptor const * fd, mp3_info * info);


#endif

