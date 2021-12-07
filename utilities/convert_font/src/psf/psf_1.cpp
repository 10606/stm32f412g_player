#include "psf_1.h"

psf_1_t::psf_1_t (std::ifstream && _psf_file, std::ofstream && _header_file) :
    psf_t(std::move(_psf_file), std::move(_header_file)),
    has_table(0)
{
    psf_file.seekg(2);
    psf_1_header_t psf_1_header;
    psf_file.read(reinterpret_cast <char *> (&psf_1_header), sizeof(psf_1_header_t));

    height = psf_1_header.font_height;
    font_size = (psf_1_header.file_mode & 1)? 512 : 256;
    has_table = (psf_1_header.file_mode & 2);
}

std::unordered_map <uint32_t, uint32_t> psf_1_t::read_map () /* symbol -> offset in file */
{
    uint32_t header_size = sizeof(psf_1_header_t) + 2;
    if (!has_table)
        return empty_map(header_size, height, font_size);
    
    std::unordered_map <uint32_t, uint32_t> ans;
    psf_file.seekg(header_size + height * font_size);
    
    for (uint32_t index = header_size, i = 0; i != font_size; )
    {
        uint16_t cur_symbol = 0xffff;
        psf_file.read(reinterpret_cast <char *> (&cur_symbol), sizeof(uint16_t));
        if (psf_file.eof())
            break;
        if (cur_symbol == 0xffff)
        {
            index += height;
            ++i;
            continue;
        }
        ans.insert({cur_symbol, index});
    }
    return ans;
}

