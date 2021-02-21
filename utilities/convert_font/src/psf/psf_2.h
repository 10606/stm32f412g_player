#ifndef PSF_2_H
#define PSF_2_H

#include <stdint.h>

#include "psf_common.h"
#include "utf8_automat.h"

struct psf_v2_header_t
{
    unsigned char magic[2]; // 0x4a 0x86
};

struct psf_2_header_t
{
    uint32_t version;
    uint32_t header_size;
    
    uint32_t flags;
    uint32_t length; // font count glyphs
    uint32_t char_size; // bytes per glyph
    uint32_t height;
    uint32_t width;
};

struct psf_2_t : public psf_t
{
public:
    psf_2_t (std::ifstream && _psf_file, std::ofstream && _header_file);

private:
    bool has_table;
    uint32_t height;
    uint32_t width;
    uint32_t font_size;
    uint32_t bytes_per_symbol;
    uint32_t header_size;
    
protected:
    virtual std::string fonst_name ()
    {
        return std::to_string(height);
    }
    
    virtual uint32_t font_width ()
    {
        return width;
    }
    
    virtual uint32_t font_height ()
    {
        return height;
    }
    
    virtual std::map <uint32_t, uint32_t> read_map (); /* symbol -> offset in file */
};


#endif

