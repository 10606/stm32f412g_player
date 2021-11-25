#ifndef SRC_INTERACTIVE_COMMON_TYPES_H
#define SRC_INTERACTIVE_COMMON_TYPES_H

#include <type_traits>
#include <stddef.h>
#include <concepts>

template <typename T>
concept has_pos = 
requires (T x)
{
    {x.pos} -> std::same_as <uint8_t &>;
};

template <typename T>
concept has_selected =
requires (T x)
{
    {x.selected} -> std::same_as <char &>;
};


template <has_pos T>
size_t get_pos (T const & value)
{
    return value.pos;
}

template <typename T>
size_t get_pos (T const &)
{
    return 0;
}


template <has_selected T>
char get_selected (T const & value)
{
    return value.selected;
}

template <typename T>
char get_selected (T const &)
{
    return 0;
}

#endif

