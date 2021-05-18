#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <map>
#include "path_conversion.h"
#include "char_conversion.h"
#include "fill_char_set.h"
#include "FAT.h"
#include "LFN.h"
#include "playlist_structures.h"

std::ifstream partition_with_FAT;
uint32_t const start_partition_sector = 1;

uint32_t read_sector (uint32_t sector_number, void * buffer)
{
    try
    {
        partition_with_FAT.seekg(static_cast <uint64_t> (sector_number) * 512);
        partition_with_FAT.read(static_cast <char *> (buffer), 512);
        return 0;
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
}

FAT_info_t FAT_info(512, start_partition_sector, read_sector);


std::pair <std::string, std::string> 
get_group_song_names 
(std::string const & value)
{
    if (value.size() < 4)
        return std::make_pair("", value);
    size_t b3_pos = value.rfind("_-_");
    if (b3_pos != std::string::npos)
        return split_string(value, b3_pos, 3);
    size_t b2_pos = value.rfind("__");
    if (b2_pos != std::string::npos)
        return split_string(value, b2_pos, 2);
    
    for (size_t i = 1; i != value.size(); ++i)
    {
        if ((value[i]   == '_' || value[i]   == '-') &&
            (value[i-1] == '_' || value[i-1] == '-'))
        {
            return split_string(value, i - 1, 2);
        }
    }
    std::string answer = value.substr(0, value.find('.'));
    return {answer, ""};
}

// utf8 -> ucs4 -> my_custom_code_table
std::basic_string <uint8_t> convert_to_custom_char_table (std::string const & value)
{
    static std::map <uint32_t, uint8_t> rev_map = fill_rev_char_map();
    std::basic_string <uint8_t> ans;
    std::vector <uint32_t> ucs4 = utf8_to_ucs4(value);
    for (uint32_t v : ucs4)
    {
        std::map <uint32_t, uint8_t> :: const_iterator it = rev_map.find(v);
        if (it == rev_map.end())
            ans += 0x20; // ' '
        else
            ans += it->second;
    }
    return ans;
}

void write (char * dst, std::string const & _src, uint32_t size, char fill = ' ')
{
    std::basic_string <uint8_t> src = convert_to_custom_char_table(_src);
    std::memcpy(dst, src.c_str(), std::min <uint32_t> (src.size(), size));
    if (src.size() < size)
        std::memset(dst + src.size(), fill, size - src.size());
}

void write_group_song_names (std::string const & value, song_header * song)
{
    std::pair <std::string, std::string> names = 
        get_group_song_names (value);
    write(song->song_name, names.second, sz::song_name);
    write(song->group_name, names.first, sz::group_name);
}


void write_header (std::ostream & out, std::string const & name, uint32_t cnt)
{
    playlist_header header;
    write(header.playlist_name, name, sz::pl_name);
    header.cnt_songs = cnt;
    out.write(reinterpret_cast <char *> (&header), sizeof(playlist_header));
}


using songs_t = std::vector <std::pair <song_header, std::unique_ptr <char[][12]> > >;

void write_songs
(
    std::ostream & out, 
    std::string const & name, 
    songs_t & songs // <header, path>
)
{
    write_header(out, name, songs.size()); 
    uint32_t offset = 0;
    for (auto & song : songs)
    {
        song.first.path_offset = 
            sizeof(playlist_header) + 
            songs.size() * sizeof(song_header) +
            offset * sizeof(char[12]);
        offset += song.first.path_len;
        out.write(reinterpret_cast <char const *> (&song.first), sizeof(song_header));
    }
    for (size_t i = 0; i != songs.size(); ++i)
        out.write(reinterpret_cast <char const *> (songs[i].second.get()), songs[i].first.path_len * sizeof(char[12]));
}


songs_t init_songs (std::vector <std::string> const & paths)
{
    songs_t answer;
    
    uint32_t offset = 0;
    for (std::string const & path : paths)
    {
        try
        {
            std::vector <std::string> s_path = split_path(path);
            converted_path l_path = make_long_path(s_path);
            std::unique_ptr <char[][12]> c_path = l_path.convert_path(&FAT_info);
            song_header song;
            write_group_song_names(s_path.back(), &song);
            song.path_len = l_path.size();
            offset += song.path_len;
            answer.emplace_back(song, std::move(c_path));
        }
        catch (std::exception const & e)
        {
            std::cerr << e.what() << " : " << path << "\n";
        }
    }
    
    return answer;
}


void write_all (std::ostream & out, std::istream & in, std::string const & name)
{
    std::vector <std::string> paths;
    for (std::string path; std::getline(in, path);)
        paths.push_back(path);   
    
    songs_t songs = init_songs(paths);
    write_songs(out, name, songs);
}


int main (int argc, char ** argv)
{
    if (argc <= 2)
    {
        std::cout << argv[0] << " <playlist_src> <playlist_dst.plb>" << std::endl;
        return 1;
    }
    
    try
    {
        std::ifstream playlist_src(argv[1]);
        std::ofstream playlist_dst(argv[2]);
        
        partition_with_FAT = std::ifstream("/dev/mmcblk0");
        partition_with_FAT.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
        
        if (FAT_info.init())
        {
            std::cerr << "init FATfs error\n";
            return 1;
        }
    
        write_all(playlist_dst, playlist_src, argv[1]);   
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
}


