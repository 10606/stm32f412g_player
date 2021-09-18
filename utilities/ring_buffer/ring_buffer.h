#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <string_view>
#include <algorithm>
#include <memory>

struct ring_buffer
{
    ring_buffer (size_t _size_inc_value = 1024) :
        data(),
        capacity(0),
        begin_v(0),
        size_v(0),
        offset_in_history(0),
        size_inc_value(_size_inc_value)
    {}
    
    ~ring_buffer () = default;
    
    void swap (ring_buffer & rhs) noexcept
    {
        std::swap(data, rhs.data);
        std::swap(capacity, rhs.capacity);
        std::swap(begin_v, rhs.begin_v);
        std::swap(size_v, rhs.size_v);
        std::swap(offset_in_history, rhs.offset_in_history);
        std::swap(size_inc_value, rhs.size_inc_value);
    }
    
    ring_buffer (ring_buffer const &) = delete;
    ring_buffer (ring_buffer && rhs) noexcept :
        ring_buffer(rhs.size_inc_value)
    {
        swap(rhs);
    }
    
    ring_buffer & operator = (ring_buffer const &) = delete;
    ring_buffer & operator = (ring_buffer && rhs) noexcept
    {
        swap(rhs);
        return *this;
    }
    
    
    // ret - diff for iterators
    size_t add (std::string_view value);
    
    void erase (size_t count)
    {
        begin_v += count;
        if (begin_v >= capacity)
            begin_v -= capacity;
        
        size_v -= count;
        offset_in_history += count;
        
        if (size_v + 2 * size_inc_value < capacity)
            realloc(0);
    }
    
    size_t write (int fd, size_t pos);
    
    char operator [] (size_t pos) const noexcept
    {
        pos -= offset_in_history;
        
        pos += begin_v;
        if (pos >= capacity)
            pos -= capacity;
        
        return data[pos];
    }
    
    size_t size () const noexcept
    {
        return size_v;
    }
    
    size_t begin () const noexcept
    {
        return offset_in_history;
    }
    
    size_t end () const noexcept
    {
        return size_v + offset_in_history;
    }
    

private:
    void realloc (size_t size_diff);

    std::unique_ptr <char []> data;
    size_t capacity;
    size_t begin_v;
    size_t size_v;
    size_t offset_in_history;
    size_t size_inc_value;
};

#endif

