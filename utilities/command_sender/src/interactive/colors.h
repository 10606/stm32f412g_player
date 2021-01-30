#ifndef COLORS_H
#define COLORS_H

#include <vector>
#include <string>

struct colors
{
    static const std::vector  //cmd
    <
        std::vector //line
        <
            std::vector //current
            <
                std::vector //s
                <
                    std::string
                >
            >
        >
    > table;
};

struct color
{
    static const std::string defaul_color; 

    static const std::string white; 
    static const std::string red; 
    static const std::string cyan;
    static const std::string green;
    static const std::string yellow;
};


#endif

