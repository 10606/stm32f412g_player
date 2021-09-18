#ifndef DIRECTION_T_H
#define DIRECTION_T_H

#include <stdint.h>

namespace directions
{

namespace fb
{
    enum type : uint8_t
    {
        forward,
        backward
    };
};

namespace np
{
    enum type : uint8_t
    {
        next,
        prev
    };
};

namespace lr
{
    enum type : uint8_t
    {
        left,
        right
    };
};

namespace du
{
    enum type : uint8_t
    {
        down,
        up
    };
};

namespace nlrud
{
    enum type : uint8_t
    {
        none,
        
        left,
        right,
        
        up,
        down
    };
};

}

#endif

