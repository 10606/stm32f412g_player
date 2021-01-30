#include "check_string.h"

#include <string>
#include <vector>
#include <limits>

size_t const cost_add = 1;
size_t const cost_replace = 2;
size_t const cost_delete = 2;
size_t const cost_swap = 2;

size_t diff_string (std::string const & a, std::string const & b)
{
    if ((a.size() * diff_sz_limit < b.size()) ||
        (b.size() * diff_sz_limit < a.size()))
    {
        return std::max(a.size(), b.size());
    }
    
    std::vector <std::vector <size_t> > distance
    (
        a.size() + 1, 
        std::vector <size_t> 
        (
            b.size() + 1, 
            std::numeric_limits <size_t> :: max()
        )
    );
    
    for (size_t i = 0; i != a.size() + 1; ++i)
    {
        distance[i][0] = i;
    }
     
    for (size_t i = 0; i != b.size() + 1; ++i)
    {
        distance[0][i] = i;
    }
    
    for (size_t i = 1; i != a.size() + 1; ++i)
    {
        for (size_t j = 1; j != b.size() + 1; ++j)
        {
            distance[i][j] = std::min(distance[i][j], distance[i - 1][j] + cost_delete);
            distance[i][j] = std::min(distance[i][j], distance[i][j - 1] + cost_add);
            distance[i][j] = std::min(distance[i][j], distance[i - 1][j - 1] + cost_replace);
            if (a[i - 1] == b[j - 1])
                distance[i][j] = std::min(distance[i][j], distance[i - 1][j - 1]);
            if ((i >= 2) &&
                (j >= 2) &&
                (a[i - 2] == b[j - 1]) &&
                (a[i - 1] == b[j - 2]))
                distance[i][j] = std::min(distance[i][j], distance[i - 2][j - 2] + cost_swap);
            
        }
    }
    
    return distance[a.size()][b.size()];
}


