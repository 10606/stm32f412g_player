#ifndef PATH_CONVERSION_H
#define PATH_CONVERSION_H

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include "FAT.h"

struct converted_path
{
    converted_path (std::vector <std::string> const & _path);
    converted_path (converted_path const &) = delete;
    converted_path & operator = (converted_path const &) = delete;
    converted_path (converted_path &&) = default;
    converted_path & operator = (converted_path &&) = default;

    std::unique_ptr <char[][12]> convert_path (FAT_info_t * FAT_info) const;
    
    size_t size () const noexcept
    {
        return path.size();
    }
    
    friend converted_path make_long_path (std::vector <std::string> const & path);
    
private:
    std::vector <uint16_t *> path;
    std::vector <std::basic_string <uint16_t> > wpath;
};

std::vector <std::string> split_path (std::string const & path);
converted_path make_long_path (std::vector <std::string> const & path);

std::pair <std::string, std::string> 
split_string
(std::string const & value, size_t pos, size_t p_size);


#endif

