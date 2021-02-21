#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <memory>

#include "psf.h"
#include "fill_char_set.h"

int main (int argc, char ** argv)
{
    try
    {
        if (argc < 2)
        {
            std::cout << argv[0] << " font_name (without .psf)\n";
            std::cout << argv[0] << " src_file_name dst_file_name\n";
            return 1;
        }
        
        std::string psf_name =
            (argc == 2)? (std::string(argv[1]) + ".psf") :
            argv[1];
        
        std::string header_name =
            (argc == 2)? (std::string(argv[1]) + ".c") :
            argv[2];
        
        std::ifstream psf_file(psf_name);
        std::ofstream header_file(header_name);
        std::unique_ptr <psf_t> psf_converter = 
            open_psf(std::move(psf_file), std::move(header_file));

        std::array <uint32_t, 256> char_set;
        fill_char_set(char_set);
        psf_converter->write_font(char_set);
    }
    catch (std::exception const & e)
    {
        std::cout << e.what();
    }
    return 0;
}


