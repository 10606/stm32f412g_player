#include "psf.h"

std::unique_ptr <psf_t> open_psf (std::ifstream && _psf_file, std::ofstream && _header_file)
{
    psf_v_header_t psf_v_header;
    _psf_file.read(reinterpret_cast <char *> (&psf_v_header), sizeof(psf_v_header_t));

    if ((psf_v_header.magic[0] == 0x36) &&
        (psf_v_header.magic[1] == 0x04)) // psf1
    {
        std::cout << "psf 1" << "\n";
        return std::make_unique <psf_1_t> 
            (std::move(_psf_file), std::move(_header_file));
    }
    else if ((psf_v_header.magic[0] == 0x72) &&
             (psf_v_header.magic[1] == 0xb5)) // psf2
    {
        std::cout << "psf 2" << "\n";
        return std::make_unique <psf_2_t> 
            (std::move(_psf_file), std::move(_header_file));
    }
    else 
    {
        throw std::runtime_error("unknown header");
    }
}


