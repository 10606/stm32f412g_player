#ifndef ID3_H
#define ID3_H

#include "FAT.h"
#include <stdint.h>

typedef struct
{
    char header[3]; //TAG
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    char genre;
} id3_v1_header;


typedef struct 
{
    char marker[3]; //ID3
    char version;
    char sub_version;
    char flags;
    char length[4]; //pidoracy
} id3_v2_header;


typedef struct 
{
    unsigned char value[4];
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
} mp3_header;


typedef struct 
{
    char header[4];
    char _[4];
    unsigned char length[4];
} xing_header;

typedef struct 
{
    char header[4];
    char _[6];
    uint16_t length_l; 
    uint16_t length_h; 
} vbri_header;


typedef struct
{
    uint32_t offset;
    float length;
} mp3_info;

void get_length (file_descriptor * fd, mp3_info * info);


#endif

