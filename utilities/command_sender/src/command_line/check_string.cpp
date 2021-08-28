#include "check_string.h"

#include <string_view>
#include <vector>
#include <limits>

size_t diff_string (std::string_view const a, std::string_view const b)
{
    static size_t const cost_add = 1;
    static size_t const cost_replace = 3;
    static size_t const cost_delete = 3;
    static size_t const cost_swap = 2;

    if ((a.size() * diff_sz_limit < b.size()) ||
        (b.size() * diff_sz_limit < a.size()))
        return std::max(a.size(), b.size());
 
    std::vector <size_t> distance_prev_2(b.size() + 1); // for swap
    std::vector <size_t> distance_prev(b.size() + 1); 
    std::vector <size_t> distance_cur(b.size() + 1); 
    
    for (size_t i = 0; i != a.size() + 1; ++i)
    {
        for (size_t j = 0; j != b.size() + 1; ++j)
        {
            if ((i == 0) || (j == 0))
            {
                distance_cur[j] = cost_delete * i + cost_add * j;
                continue;
            }
            distance_cur[j] = std::numeric_limits <size_t> :: max();
            distance_cur[j] = std::min(distance_cur[j], distance_prev[j] + cost_delete);
            distance_cur[j] = std::min(distance_cur[j], distance_cur[j - 1] + cost_add);
            distance_cur[j] = std::min(distance_cur[j], distance_prev[j - 1] + cost_replace);
            if (a[i - 1] == b[j - 1])
                distance_cur[j] = std::min(distance_cur[j], distance_prev[j - 1]);
            if ((i >= 2) &&
                (j >= 2) &&
                (a[i - 2] == b[j - 1]) &&
                (a[i - 1] == b[j - 2]))
                distance_cur[j] = std::min(distance_cur[j], distance_prev_2[j - 2] + cost_swap);
            
        }
        // roll distances
        std::swap(distance_prev_2, distance_cur);
        std::swap(distance_prev_2, distance_prev);
    }
    
    return distance_prev[b.size()];
}


