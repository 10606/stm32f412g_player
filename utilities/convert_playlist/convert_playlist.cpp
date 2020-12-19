extern "C"
{
#include "FAT.h"
#include "LFN.h"
#include "playlist_structures.h"
}

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <algorithm>

extern "C"
{
    std::ifstream partition_with_FAT;
    uint32_t start_partition_sector = 1;

    uint32_t read_sector (uint32_t sector_number, void * buffer)
    {
        try
        {
            partition_with_FAT.seekg
            (
                static_cast <uint64_t> (sector_number) *
                512//static_cast <uint64_t> (global_info.sector_size)
            );
    
            partition_with_FAT.read
            (
                static_cast <char *> (buffer),
                512//global_info.sector_size
            );
            if (!partition_with_FAT.good())
            {
                std::cerr << "bad \n";
                return 1;
            }
            return 0;
        }
        catch (std::exception const & e)
        {
            std::cerr << e.what() << "\n";
            return 1;
        }
    }
}


std::vector <std::string> split_path (std::string const & path)
{
    std::vector <std::string> answer;
    size_t old_pos = 0;
    
    while (1)
    {
        size_t new_pos;
        if ((new_pos = path.find('/', old_pos)) != std::string::npos)
        {
            answer.push_back(path.substr(old_pos, new_pos - old_pos));
            old_pos = new_pos + 1;
        }
        else
        {
            answer.push_back(path.substr(old_pos));
            return answer;
        }
        if (old_pos >= path.size())
        {
            return answer;
        }
    }
}


struct converted_path
{
    converted_path (size_t count, size_t size) :
        path(count, nullptr),
        storage(new char [count * size])
    {
        for (size_t i = 0; i != path.size(); ++i)
        {
            path[i] = storage.get() + i * size;
        }
    }
    
    converted_path (converted_path const &) = delete;
    converted_path (converted_path &&) = default;
    
    std::vector <char *> path;
    
private:
    std::unique_ptr <char[]> storage;
};

template <typename Char_T>
std::basic_string <Char_T> utf8_to_utf16 (std::string const & utf8)
{
    std::vector <uint32_t> unicode;
    size_t i = 0;
    while (i < utf8.size())
    {
        uint32_t uni;
        size_t todo;
        unsigned char ch = utf8[i++];
        if (ch <= 0x7F)
        {
            uni = ch;
            todo = 0;
        }
        else if (ch <= 0xBF)
        {
            throw std::logic_error("not a UTF-8 string");
        }
        else if (ch <= 0xDF)
        {
            uni = ch & 0x1F;
            todo = 1;
        }
        else if (ch <= 0xEF)
        {
            uni = ch & 0x0F;
            todo = 2;
        }
        else if (ch <= 0xF7)
        {
            uni = ch & 0x07;
            todo = 3;
        }
        else
        {
            throw std::logic_error("not a UTF-8 string");
        }
        for (size_t j = 0; j < todo; ++j)
        {
            if (i == utf8.size())
                throw std::logic_error("not a UTF-8 string");
            unsigned char ch = utf8[i++];
            if (ch < 0x80 || ch > 0xBF)
                throw std::logic_error("not a UTF-8 string");
            uni <<= 6;
            uni += ch & 0x3F;
        }
        if (uni >= 0xD800 && uni <= 0xDFFF)
            throw std::logic_error("not a UTF-8 string");
        if (uni > 0x10FFFF)
            throw std::logic_error("not a UTF-8 string");
        unicode.push_back(uni);
    }
    std::basic_string <Char_T> utf16;
    for (size_t i = 0; i < unicode.size(); ++i)
    {
        uint32_t uni = unicode[i];
        if (uni <= 0xFFFF)
        {
            utf16 += (Char_T)uni;
        }
        else
        {
            uni -= 0x10000;
            utf16 += (Char_T)((uni >> 10) + 0xD800);
            utf16 += (Char_T)((uni & 0x3FF) + 0xDC00);
        }
    }
    return utf16;
}
/*
inline std::string utf8_to_utf16 (std::string const & value)
{
    //TODO
    std::string answer;
    for (char c : value)
    {
        answer += (c);
        answer += (char)0;
    }
    return answer;
}
*/

template <typename Char_T>
converted_path make_long_path (std::vector <std::string> const & path)
{
    std::vector <std::basic_string <Char_T> > wpath;
    for (std::string const & p : path)
    {
        wpath.push_back(utf8_to_utf16 <Char_T> (p));
    }
    size_t max_size = 0;
    for (std::basic_string <Char_T> const & p : wpath)
    {
        max_size = std::max(max_size, p.size());
    }
    max_size *= sizeof(Char_T);
    converted_path answer(path.size(), max_size + 2);
    for (size_t i = 0; i != wpath.size(); ++i)
    {
        std::memcpy(answer.path[i], reinterpret_cast <char const *> (wpath[i].c_str()), sizeof(Char_T) * (wpath[i].size() + 1));
    }
    return answer;
}

std::unique_ptr <char[][12]> convert_path (converted_path const & path)
{
    std::unique_ptr <char[][12]> answer(new char [path.path.size()][12]);
    file_descriptor fd;
    uint32_t ret;
    if ((ret = open_lfn(&fd, path.path.data(), answer.get(), path.path.size())) != 0)
    {
        throw std::runtime_error(std::to_string(ret) + " while open file");
    }
    return answer;
}

std::pair <std::string, std::string> 
get_group_song_names 
(std::string const & value)
{
    if (value.size() < 4)
    {
        return std::make_pair(value, "");
    }
    for (size_t i = 1; i != value.size(); ++i)
    {
        if ((value[i]   == '_' || value[i]   == '-') &&
            (value[i-1] == '_' || value[i-1] == '-'))
        {
            if (i + 1 < value.size())
            {
                std::string group = value.substr(0, i - 1);
                std::string song = value.substr(i + 1, value.find('.', i+1) - i - 1);
                return std::make_pair(group, song);
            }
            else
            {
                return std::make_pair(value.substr(0, i-1), "");
            }
        }
    }
    std::string answer = value.substr(0, value.find('.'));
    return std::make_pair <std::string const &, char const (&) [1]> (answer, "");
}

void write (char * dst, std::string const & src, uint32_t size, char fill = ' ')
{
    std::memcpy(dst, src.c_str(), std::min <uint32_t> (src.size(), size));
    if (src.size() < size)
    {
        std::memset(dst + src.size(), fill, size - src.size());
    }
}

void write_group_song_names (std::string const & value, song_header * song)
{
    std::pair <std::string, std::string> names = 
        get_group_song_names (value);
    write(song->song_name, names.second, song_name_sz);
    write(song->group_name, names.first, group_name_sz);
}


void write_header (std::ostream & out, std::string const & name, uint32_t cnt)
{
    playlist_header header;
    write(header.playlist_name, name, pl_name_sz);
    header.cnt_songs = cnt;
    out.write(reinterpret_cast <char *> (&header), sizeof(playlist_header));
}

void write_all (std::ostream & out, std::istream & in, std::string const & name)
{
    std::vector <std::string> paths;
    for (std::string path; std::getline(in, path);)
    {
        paths.push_back(path);   
    }
    
    std::vector <song_header> songs;
    std::vector <std::unique_ptr <char[][12]> > c_paths;
    {
        uint32_t offset = 0;
        for (std::string const & path : paths)
        {
            try
            {
                std::vector <std::string> s_path = split_path(path);
                converted_path l_path = make_long_path <int16_t> (s_path);
                std::unique_ptr <char[][12]> c_path = convert_path(l_path);
                song_header song;
                write_group_song_names(s_path.back(), &song);
                song.path_len = l_path.path.size();
                offset += song.path_len;
                songs.push_back(song);
                c_paths.push_back(std::move(c_path));
            }
            catch (std::exception const & e)
            {
                std::cerr << e.what() << " : " << path << "\n";
            }
        }
    }
    
    {
        write_header(out, name, songs.size()); 
        uint32_t offset = 0;
        for (song_header & song : songs)
        {
            song.path_offset = 
                sizeof(playlist_header) + 
                songs.size() * sizeof(song_header) +
                offset * sizeof(char[12]);
            offset += song.path_len;
            out.write(reinterpret_cast <char const *> (&song), sizeof(song_header));
        }
        for (size_t i = 0; i != c_paths.size(); ++i)
        {
            out.write(reinterpret_cast <char *> (c_paths[i].get()), songs[i].path_len * sizeof(char[12]));
        }
    }
}

int main (int argc, char ** argv)
{
    if (argc <= 2)
    {
        std::cout << argv[0] << "<playlist_src> <playlist_dst.plb>" << std::endl;
        return 1;
    }
    
    std::ifstream playlist_src(argv[1]);
    std::ofstream playlist_dst(argv[2]);
    
    partition_with_FAT = std::ifstream("/dev/mmcblk0");
    
    if (init_fatfs())
    {
        std::cerr << "init FATfs error\n";
        return 1;
    }
    
    try
    {
        write_all(playlist_dst, playlist_src, argv[1]);   
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << "\n";
    }
}

