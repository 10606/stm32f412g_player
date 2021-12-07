#ifndef PSF_1_H
#define PSF_1_H

#include "psf_common.h"

struct psf_1_header_t
{
    char file_mode;
    /*
        0 : 256, no unicode data
        1 : 512, no unicode data
        2 : 256, with unicode data 
        3 : 512, with unicode data
    */
    unsigned char font_height;
};


struct psf_1_t : public psf_t
{
public:
    psf_1_t (std::ifstream && _psf_file, std::ofstream && _header_file);

private:
    bool has_table;
    uint32_t height;
    uint32_t font_size;
    
protected:
    virtual std::string fonst_name ()
    {
        return std::to_string(height);
    }
    
    virtual uint32_t font_width () const noexcept
    {
        return 8;
    }
    
    virtual uint32_t font_height () const noexcept
    {
        return height;
    }
    
    std::unordered_map <uint32_t, uint32_t> read_map (); /* symbol -> offset in file */
};

#endif

