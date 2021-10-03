#include "ring_buffer.h"

#include <stdexcept>
#include <memory.h>
#include <limits>

#include <unistd.h>
#include <errno.h>

size_t ring_buffer::add (std::string_view value)
{
    size_t ans = 0;
    if (capacity - size_v < value.size())
        realloc(value.size());
    
    size_t end = begin_v + size_v;
    if (end >= capacity)
        end -= capacity;
    size_t tail = capacity - end;
    if (value.size() > tail)
    {
        memcpy(data.get() + end, value.data(), tail);
        memcpy(data.get(), value.data() + tail, value.size() + - tail);
    }
    else
    {
        memcpy(data.get() + end, value.data(), value.size());
    }
    size_v += value.size();
    
    if ((offset_in_history + size_v >= std::numeric_limits <size_t> :: max() / 2) ||
        (offset_in_history + size_v < offset_in_history))
    {
        ans = offset_in_history;
        offset_in_history = 0;
    }
    
    return ans;
}

void ring_buffer::realloc (size_t size_diff)
{
    size_t new_capacity = size_v + size_diff + size_inc_value;
    char * new_data = new char [new_capacity]; 

    size_t head = capacity - begin_v;
    if (size_v > head)
    {
        memmove(new_data, data.get() + begin_v, head);
        memmove(new_data + head, data.get(), size_v - head);
    }
    else
    {
        memmove(new_data, data.get() + begin_v, size_v);
    }
    
    begin_v = 0;
    capacity = new_capacity;
    data.reset(new_data);
}

