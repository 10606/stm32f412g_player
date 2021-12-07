#include "psf_2.h"

psf_2_t::psf_2_t (std::ifstream && _psf_file, std::ofstream && _header_file) :
    psf_t(std::move(_psf_file), std::move(_header_file)),
    has_table(0)
{
    psf_file.seekg(2);
    psf_v2_header_t psf_v2_header;
    psf_file.read(reinterpret_cast <char *> (&psf_v2_header), sizeof(psf_v2_header_t));
    if (psf_v2_header.magic[0] != 0x4a || psf_v2_header.magic[1] != 0x86)
        throw std::runtime_error("wrong psf2 header");
    
    psf_2_header_t psf_2_header;
    psf_file.read(reinterpret_cast <char *> (&psf_2_header), sizeof(psf_2_header_t));

    has_table = (psf_2_header.flags & 1);
    height = psf_2_header.height;
    width = psf_2_header.width;
    font_size = psf_2_header.length;
    bytes_per_symbol = psf_2_header.char_size;
    header_size = psf_2_header.header_size;
}

std::unordered_map <uint32_t, uint32_t> psf_2_t::read_map () /* symbol -> offset in file */
{
    if (!has_table)
        return empty_map(header_size, bytes_per_symbol, font_size);
    
    psf_file.seekg(header_size + bytes_per_symbol * font_size);
    
    std::unordered_map <uint32_t, uint32_t> ans;
    utf8_automat aut;
    
    for (uint32_t index = header_size, i = 0; i != font_size; )
    {
        char cur_symbol;
        psf_file.get(cur_symbol);
        if (psf_file.eof())
            break;
        utf8_automat::state_t ret = aut.next(cur_symbol);
        switch (ret)
        {
        case utf8_automat::none:
            break;
        case utf8_automat::err:
            index += bytes_per_symbol;
            ++i;
            continue;
        case utf8_automat::ready:
            ans.insert({aut.get_ans(), index});
            break;
        }
    }
    return ans;
}

