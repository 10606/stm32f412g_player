#include "path_conversion.h"

#include <vector>
#include <string>
#include <cstring>
#include <memory>
#include <set>
#include <stdexcept>
#include "char_conversion.h"
#include "FAT.h"
#include "LFN.h"

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
            return answer;
    }
}


converted_path::converted_path (std::vector <std::string> const & _path) :
    path(_path.size(), nullptr),
    wpath()
{
    for (std::string const & p : _path)
        wpath.push_back(utf8_to_ucs2(p));
    for (size_t i = 0; i != path.size(); ++i)
        path[i] = wpath[i].data();
}


std::unique_ptr <char[][12]> converted_path::convert_path (FAT_info_t * FAT_info) const
{
    std::unique_ptr <char[][12]> answer(new char [path.size()][12]);
    file_descriptor fd;
    uint32_t ret;
    if ((ret = open_lfn(FAT_info, &fd, path.data(), answer.get(), path.size())) != 0)
        throw std::runtime_error(std::to_string(ret) + " while open file");

    return answer;
}


std::string remove_bad (std::string const & value, std::vector <std::string> const & bad)
{
    if (bad.empty())
        return value;

    std::string answer;
    std::set <std::pair <size_t, size_t> > to_remove; // pos, index
    for (size_t i = 0; i != bad.size(); ++i)
        to_remove.insert({value.find(bad[i]), i});
    
    for (size_t i = 0; i < value.size(); )
    {
        std::set <std::pair <size_t, size_t> > ::iterator it = to_remove.begin();
        size_t new_i = i;
        while (it->first <= i)
        {
            size_t pos = it->first;
            size_t index = it->second;
            to_remove.erase(it);
            to_remove.insert({value.find(bad[index], pos + 1), index});
            
            if (pos + bad[index].size() > new_i)
                new_i = pos + bad[index].size();
        }
        
        if (new_i == i)
            answer += value[i++];
        else
            i = new_i;
    }
    
    return answer;
}


std::vector <std::string> const bad_in_name =
{
    "(promusic.me)",
    "muzlostyle.ru",
    "(larkabout.net)",
    "(mp3CC.biz)",
    "(zaycev.net)",
    "(mp3no.com)",
    "(playmus.cc)",
    "(playvk.com)",
    "(44outdoors.com)",
    "(seehall.me)",
    "(zoop.su)"
};


std::pair <std::string, std::string> 
split_string
(std::string const & value, size_t pos, size_t p_size)
{
    std::string group;
    std::string song;
    
    if (pos + p_size < value.size())
    {
        size_t pos_2 = pos + p_size;
        size_t end_pos = value.rfind('.');
        if (end_pos < pos_2)
            end_pos = std::string::npos;
        song = value.substr(pos_2, end_pos - pos_2);
    }
    else
        song = "";
    
    if (pos >= 1)
        group = value.substr(0, pos);
    else
        group = "";
    
    return std::make_pair(remove_bad(group, bad_in_name), remove_bad(song, bad_in_name));
}

