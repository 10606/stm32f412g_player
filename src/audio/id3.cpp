#include "id3.h"

#include "FAT.h"
#include "util.h"
#include <stdint.h>
#include <string.h>

static inline char is_eq (char const * a, char const * b, uint32_t length)
{
    for (uint32_t i = 0; i != length; ++i)
    {
        if (a[i] != b[i])
            return 0;
    }
    return 1;
}


static inline uint32_t id3_v1_size (file_descriptor * fd)
{
    ret_code ret;
    if ((ret = fd->seek(0)))
        return 0;
    
    id3_v1_header id3_v1_h;
    uint32_t rb;
    uint32_t total_read = 0;
    
    while (total_read < 3)
    {
        if ((ret = fd->read((char *)&id3_v1_h + total_read, sizeof(id3_v1_header) - total_read, &rb)))
            return 0;
        total_read += rb;
    }
    if (is_eq(id3_v1_h.header, "TAG", 3))
        return sizeof(id3_v1_header);
    else
        return 0;
}

static inline uint32_t id3_v2_size (file_descriptor * fd)
{
    ret_code ret;
    if ((ret = fd->seek(0)))
        return 0;
    
    id3_v2_header id3_v2_h;
    uint32_t rb;
    uint32_t total_read = 0;
    
    while (total_read < sizeof(id3_v2_header))
    {
        if ((ret = fd->read((char *)&id3_v2_h + total_read, sizeof(id3_v2_header) - total_read, &rb)))
            return 0;
        total_read += rb;
    }

    if (is_eq(id3_v2_h.marker, "ID3", 3))
    {
        uint32_t length = 0;
        length += (uint32_t)id3_v2_h.length[0] << 21;
        length += (uint32_t)id3_v2_h.length[1] << 14;
        length += (uint32_t)id3_v2_h.length[2] << 7;
        length += (uint32_t)id3_v2_h.length[3] << 0;
        return length + sizeof(id3_v2_header);
    }
    else
    {
        return 0;
    }
}


//res = offset of header
//0xffffffff = not found
uint32_t get_mp3_header (file_descriptor * fd, mp3_header * mp3_h, uint32_t id3_length)
{
    ret_code ret;
    uint32_t rb;
    if ((ret = fd->seek(id3_length)))
        return 0xffffffff;
    
    mp3_h->value[0] = 0;
    mp3_h->value[1] = 0;
    
    while ((mp3_h->value[0] != 0xff) || 
          ((mp3_h->value[1] &  0xe0) != 0xe0))
    {
        mp3_h->value[0] = mp3_h->value[1];
        do
        {
            if ((ret = fd->read(mp3_h->value + 1, 1, &rb)))
                return 0xffffffff;
        } 
        while (rb == 0);
    }
    uint32_t total_read = 2;
    while (total_read < sizeof(mp3_header))
    {
        if ((ret = fd->read((char *)mp3_h->value + total_read, sizeof(mp3_header) - total_read, &rb)))
            return 0xffffffff;
        total_read += rb;
    }

    return fd->current_position() - sizeof(mp3_header);
}


uint32_t get_xing_length (file_descriptor * fd, mp3_header * mp3_h, uint32_t mp3_header_pos)
{
    uint32_t offset = 
        mp3_header_pos + 
        4 + 
        (((mp3_h->value[3] >> 6)/*24..25 -> channel_mode*/ == 3)? 17 : 32);
    ret_code ret;

    if ((ret = fd->seek(offset)))
        return 0;
    
    xing_header xing;
    uint32_t total_read = 0;
    while (total_read < sizeof(xing_header))
    {
        uint32_t rb;
        if ((ret = fd->read((char *)&xing + total_read, sizeof(xing_header) - total_read, &rb)))
            return 0;
        total_read += rb;
    }
    
    if (is_eq(xing.header, "Xing", 4) ||
        is_eq(xing.header, "Info", 4))
    {
        uint32_t length = 0;
        length |= (uint32_t)xing.length[0] << 24;
        length |= (uint32_t)xing.length[1] << 16;
        length |= (uint32_t)xing.length[2] << 8;
        length |= (uint32_t)xing.length[3] << 0;
        return length;
    }
    else
    {
        return 0;
    }
}

uint32_t get_vbri_length (file_descriptor * fd, mp3_header * mp3_h, uint32_t mp3_header_pos)
{
    uint32_t offset = 
        mp3_header_pos + 
        4 + 
        32;
    ret_code ret;

    if ((ret = fd->seek(offset)))
        return 0;
    
    vbri_header vbri;
    uint32_t total_read = 0;
    while (total_read < sizeof(vbri_header))
    {
        uint32_t rb;
        if ((ret = fd->read((char *)&vbri + total_read, sizeof(vbri_header) - total_read, &rb)))
            return 0;
        total_read += rb;
    }
    
    if (is_eq(vbri.header, "VBRI", 4))
       return ((uint32_t)vbri.length_h << 16) + vbri.length_l;
    else
        return 0;
}


void get_length (file_descriptor const * _fd, mp3_info * info)
{
    file_descriptor fd(*_fd, 0);

    static uint32_t bitrate_index[] = 
    { //MPEG-1 layer 3 bitrate
        1,      32,     40,     48,     56,     64,     80,     96,
        112,    128,    160,    192,    224,    256,    320,    1
    };
    
    static float frequency_index[] =
    { //MPEG-1 layer 3 frequency
        44100, 48000, 32000, 44100
    };

    uint32_t id3_size = id3_v1_size(&fd) + id3_v2_size(&fd);
    mp3_header mp3_h;

    uint32_t mp3_offset = get_mp3_header(&fd, &mp3_h, id3_size);
    uint32_t vbr_length = 0;
    if (mp3_offset == 0xffffffff)
    {
        info->offset = id3_size;
        info->length = 60 * 1000;
    }
    else
    {   
        info->offset = mp3_offset;
        vbr_length =
            get_vbri_length(&fd, &mp3_h, mp3_offset) +
            get_xing_length(&fd, &mp3_h, mp3_offset);
        if (vbr_length == 0)
        {
            info->length = (float)(fd.size - id3_size) /
                (float)bitrate_index[(uint32_t)mp3_h.value[2] >> 4/*16..19 -> bitrate*/] * (float)8;
        }
        else
        {
            info->length = (float)vbr_length * (float)1152 /*sample per frame*/ * 1000 /
                frequency_index[((uint32_t)mp3_h.value[2] >> 2) & 0b11/*20..21 -> freq*/];
        }
    }
}




