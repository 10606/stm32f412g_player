#include "psf_common.h"


std::ostream & print_header (std::ostream & s, std::string const & font_name)
{
    s << "\n#include \"fonts.h\"\n\n"; 
    s << "uint8_t const " 
      << "f_" << font_name << "_table"
      << " [] = \n{\n\n"; 
    
    return s;
}


std::ostream & print_tail 
(
    std::ostream & s, 
    std::string const & font_name,
    uint32_t font_width,
    uint32_t font_height
)
{
    s << "};\n\n\n" 
      << "sFONT font_" << font_name
      << " = \n{\n"
      << "    " << "f_" << font_name << "_table,\n"
      << "    " << font_width << ", // width\n"
      << "    " << font_height << ", // height\n};\n\n";
    return s;
}


std::ostream & print_in_hex (std::ostream & s, char c)
{
    uint8_t v = c;
    static const char * table = "0123456789abcdef";
    s << "0x" << table[(v >> 4) & 0x0f] << table[v & 0x0f];
    return s;
}

std::ostream & print_ascii_art (std::ostream & s, char c)
{
    for (size_t j = 8; j-- != 0;)
        s << ((c & (1 << j))? '#' : ' ');
    return s;
}



void psf_t::write_font (std::array <uint32_t, 256> const & symbols)
{
    std::map <uint32_t, uint32_t> char_map = read_map();
    print_header(header_file, fonst_name());

    for (uint32_t symbol : symbols)
    {
        header_file << "// 0x" << std::hex << symbol << std::dec;
        std::map <uint32_t, uint32_t> :: iterator it = char_map.find(symbol);
        if (it == char_map.end())
        {
            header_file << "  ((\n";
            std::cout << " 0x" << std::hex << symbol << std::dec << " not found\n";
            write_none_symbol();
        }
        else
        {
            header_file << "  ok\n";
            write_symbol(it->second);
        }
    }
    
    print_tail(header_file, fonst_name(), font_width(), font_height());
}

void psf_t::write_symbol (char (psf_t::* next_symbol) ())
{
    uint32_t height = font_height();
    uint32_t byte_width = (font_width() + 7) / 8;
    
    for (uint32_t line = 0; line != height; ++line)
    {
        std::stringstream ascii_art;
        
        for (uint32_t byte_index = 0; byte_index != byte_width; ++byte_index)
        {
            char c = (this->*next_symbol)();
            print_in_hex(header_file, c) << ", ";
            print_ascii_art(ascii_art, c);
        }
        header_file << "// " << ascii_art.str();
        header_file << '\n';
    }
    header_file << '\n';
}


