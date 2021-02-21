#ifndef PSF_COMMON_H
#define PSF_COMMON_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <sstream>

struct psf_v_header_t
{
    unsigned char magic[2];
    /*
        psf1
            0x36 0x04
        psf2
            0x72 0xb5
    */
};

std::ostream & print_header (std::ostream & s, std::string const & font_name);

std::ostream & print_tail 
(
    std::ostream & s, 
    std::string const & font_name,
    uint32_t font_width,
    uint32_t font_height
);

std::ostream & print_in_hex (std::ostream & s, char c);
std::ostream & print_ascii_art (std::ostream & s, char c);


struct psf_t
{
public:
    psf_t (std::ifstream && _psf_file, std::ofstream && _header_file) :
        psf_file(std::move(_psf_file)),
        header_file(std::move(_header_file))
    {}
    
    virtual ~psf_t () = default;
    void write_font (std::array <uint32_t, 256> const & symbols);

protected:
    std::ifstream psf_file;
    std::ofstream header_file;

    virtual std::string fonst_name () = 0;
    virtual uint32_t font_width () = 0;
    virtual uint32_t font_height () = 0;
    
    virtual std::map <uint32_t, uint32_t> read_map () = 0; /* symbol -> offset in file */
    
    std::map <uint32_t, uint32_t> empty_map 
    (
        uint32_t header_size, 
        uint32_t bytes_per_symbol, 
        uint32_t font_size
    ) /* symbol -> offset in file */
    {
        std::map <uint32_t, uint32_t> ans;
        for (uint32_t i = 0, offset = header_size; (i != font_size) && (i != 128); ++i, offset += bytes_per_symbol)
            ans.insert({i, offset});
        return ans;
    }
    
private:
    char next_symbol_from_file ()
    {
        char c;
        psf_file.get(c);
        return c;
    }
    
    char next_symbol_ff ()
    {
        return -1;
    }
    
    void write_symbol (char (psf_t::* next_symbol) ());
    
    void write_none_symbol ()
    {
        write_symbol(&psf_t::next_symbol_ff);
    }
    
    void write_symbol (uint32_t offset)
    {
        psf_file.seekg(offset);
        write_symbol(&psf_t::next_symbol_from_file);
    }
};


#endif

