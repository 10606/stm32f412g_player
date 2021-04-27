#ifndef PATH_CONVERSION_H
#define PATH_CONVERSION_H

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include "FAT.h"

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
    std::vector <char *> path;
    std::unique_ptr <char[]> storage;
};

std::vector <std::string> split_path (std::string const & path);
converted_path make_long_path (std::vector <std::string> const & path);

std::pair <std::string, std::string> 
split_string
(std::string const & value, size_t pos, size_t p_size);


#endif

