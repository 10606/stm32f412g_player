#ifndef SRC_INTERACTIVE_COMMON_TYPES_H
#define SRC_INTERACTIVE_COMMON_TYPES_H

#include <type_traits>
#include <stddef.h>

template <typename U, typename = void>
struct has_pos
{
    static const bool value = 0;
};

template <typename U>
struct has_pos <U, std::enable_if_t <std::is_same_v <uint8_t, decltype(U::pos)> > >
{
    static const bool value = 1;
};


template <typename U, typename = void>
struct has_selected
{
    static const bool value = 0;
};

template <typename U>
struct has_selected <U, std::enable_if_t <std::is_same_v <char, decltype(U::selected)> > >
{
    static const bool value = 1;
};


template <typename T>
size_t get_pos (T const & value, std::enable_if_t <has_pos <T> :: value, bool> = 0)
{
    return value.pos;
}

template <typename T>
size_t get_pos (T const &, std::enable_if_t <!has_pos <T> :: value, bool> = 0)
{
    return 0;
}


template <typename T>
char get_selected (T const & value, std::enable_if_t <has_selected <T> :: value, bool> = 0)
{
    return value.selected;
}

template <typename T>
char get_selected (T const &, std::enable_if_t <!has_selected <T> :: value, bool> = 0)
{
    return 0;
}

#endif

