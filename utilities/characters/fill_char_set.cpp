#include "fill_char_set.h"

void fill_char_set (std::array <uint32_t, 256> & char_set)
{
    static uint32_t head [32] =
    {
        0x0000, // 
        0x0009, // <tab>
    
        0x0401, // Ё
        0x0451, // ё
        0x2302, // ⌂
        0x00b7, // ·
        0x2022, // •
        0x25a0, // ■
        0x25c6, // ◆
        0x00a4, // ¤
        0x00a6, // ¦
        0x00ab, // «
        0x00bb, // »
        0x00b0, // °
        0x00b1, // ±
        0x2191, // ↑
        0x2193, // ↓
        0x25c0, // ◀
        0x25b6, // ▶
        0x2591, // ░
        0x2592, // ▒
        0x2588, // █
        
        // borders
        0x2502, // │
        0x250c, // ┌
        0x2510, // ┐
        0x2514, // └
        0x2518, // ┘
        0x253c, // ┼
        0x252c, // ┬
        0x2534, // ┴
        0x2524, // ┤
        0x251c, // ├
    };
    for (uint32_t i = 0; i != 32; ++i)
        char_set[i] = head[i];
    
    // ascii
    for (uint32_t i = 32; i != 128; ++i)
        char_set[i] = i;
    
    // big russian without Ё
    for (uint32_t i = 0; i != 32; ++i)
        char_set[i + 128] = i + 0x410;
    
    // little russian without ё
    for (uint32_t i = 0; i != 32; ++i)
        char_set[i + 128 + 32] = i + 0x430;
    
    static uint32_t tail [64] =
    {
        0x201e, // „
        0x2026, // …
        0x23bb, // ⎻
        0x23bc, // ⎼
        // 60 free symbols
    };
    for (uint32_t i = 0; i != 64; ++i)
        char_set[i + 192] = tail[i];
}

std::map <uint32_t, uint8_t> rev_char_map (std::array <uint32_t, 256> const & char_set)
{
    std::map <uint32_t, uint8_t> ans;
    uint8_t index = 0;
    do
    {
        ans[char_set[index]] = index;
        index++;
    }
    while (index != 0);
    return ans;
}

std::map <uint32_t, uint8_t> fill_rev_char_map ()
{
    std::array <uint32_t, 256> char_set;
    fill_char_set(char_set);
    return rev_char_map(char_set);
}

