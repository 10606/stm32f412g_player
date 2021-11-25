#include <filesystem>
#include <iostream>
#include <fstream>

void print_addr (std::ostream & out, char * pic_ptr)
{
    out << "    reinterpret_cast <void const *> (" << static_cast <void *> (pic_ptr) << "),\n";
}

int main (int argc, char ** argv)
{
    if (argc < 3)
    {
        std::cout << argv[0] << " picture_offsets.cpp <song_pic_count> a.ch565 b.ch565 ...\n";
        return 1;
    }
    
    static const char type[] = "const void * const picture_info_t::";
    
    char * pic_ptr = reinterpret_cast <char *> (0x08040000);
    std::ofstream out(argv[1]);
    
    size_t song_pic_cnt;
    try
    {
        song_pic_cnt = std::min(std::stoi(argv[2]), argc - 3);
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what();
        return 2;
    }
    
    out << "// auto generated\n";
    out << "#include \"display_picture.h\"\n\n";
    out << "#include <stdint.h>\n\n";
    out << "namespace display\n{\n\n";
    
    out << type << "song[" << song_pic_cnt << "] =\n";
    out << "{\n";
    for (size_t i = 0; i != song_pic_cnt; ++i)
    {
        print_addr(out, pic_ptr);
        pic_ptr += std::filesystem::file_size(std::filesystem::path(argv[i + 3]));
    }
    out << "};\n";
    
    
    out << type << "err[" << (argc - song_pic_cnt - 3) << "] =\n";
    out << "{\n";
    for (size_t i = song_pic_cnt + 3; i != static_cast <size_t> (argc); ++i)
    {
        print_addr(out, pic_ptr);
        pic_ptr += std::filesystem::file_size(std::filesystem::path(argv[i]));
    }
    out << "};\n";
    
    out << "\n}\n";
}

