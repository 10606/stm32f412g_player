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
#include "convert_custom.h"

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


std::pair <std::string, std::string>  // <group, song>
get_group_song_names 
(std::string const & value)
{
    if (value.size() < 4)
        return {value, ""}; 
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
    
    std::string answer = remove_bad(value); 
    answer = answer.substr(0, answer.find('.'));
    return {answer, ""}; // group_song_name.mp3
}

void write (char * dst, std::string const & _src, uint32_t size, char fill = ' ')
{
    std::basic_string <uint8_t> src = utf8_to_custom(_src);
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

struct song_t
{
    song_t (song_header _header, std::unique_ptr <filename_t []> && _file_name) :
        header(std::move(_header)),
        file_name(std::move(_file_name))
    {}

    song_header header;
    std::unique_ptr <filename_t []> file_name;
};

using songs_t = std::vector <song_t>;

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
        song.header.path_offset = 
            sizeof(playlist_header) + 
            songs.size() * sizeof(song_header) +
            offset * sizeof(filename_t);
        offset += song.header.path_len;
        out.write(reinterpret_cast <char const *> (&song.header), sizeof(song_header));
    }
    for (size_t i = 0; i != songs.size(); ++i)
        out.write(reinterpret_cast <char const *> (songs[i].file_name.get()), songs[i].header.path_len * sizeof(filename_t));
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
            converted_path l_path(s_path);
            std::unique_ptr <filename_t []> c_path = l_path.convert_path(&FAT_info);
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
    if (argc <= 3)
    {
        std::cout << argv[0] << " <device_name> <playlist_src> <playlist_dst.plb>" << std::endl;
        return 1;
    }
    
    try
    {
        std::ifstream playlist_src(argv[2]);
        std::ofstream playlist_dst(argv[3]);
        
        partition_with_FAT = std::ifstream(argv[1]);
        partition_with_FAT.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
        
        if (FAT_info.init())
        {
            std::cerr << "init FATfs error\n";
            return 1;
        }
    
        write_all(playlist_dst, playlist_src, argv[2]);   
    }
    catch (std::exception const & e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
}


