#include <iostream>
#include <fstream>

struct lz4_header
{
    uint8_t repeat : 4, size : 4;
};

void compress (std::istream & in, std::ostream & out)
{
    while (in)
    {
        char buff[240 * sizeof(uint16_t) * 5];
        
        in.read(buff, sizeof(buff));
        size_t rb = in.gcount();
        
        std::pair <lz4_header, size_t> best[std::extent_v <decltype(buff)> + 1];
        best[rb] = {{1, 0}, 0};
        
        for (size_t pos = rb; pos--; )
        {
            best[pos] = {{1, 1}, 2 + best[pos + 1].second};
            for (uint8_t size = 1; (size != 16) && (pos + size < rb); ++size)
            {
                uint8_t repeat;
                for (repeat = 1; (repeat != 15) && (pos + size * (repeat + 1) < rb); ++repeat)
                {
                    // check next frame
                    bool good = 1;
                    for (size_t i = 0; i != size; ++i)
                    {
                        if (buff[pos + i] != buff[pos + size * repeat + i]) 
                        {
                            good = 0;
                            break;
                        }
                    }
                    
                    if (!good)
                        break;
                }
            
                size_t cur_size = 1 + size + best[pos + size * repeat].second;
                if (best[pos].second > cur_size)
                    best[pos] = {{repeat, size}, cur_size};
            }
        }
        
        for (size_t pos = 0; pos != rb; )
        {
            lz4_header cur = best[pos].first;
            out.write(reinterpret_cast <char *> (&cur), sizeof(cur));
            out.write(buff + pos, cur.size);
            pos += cur.size * cur.repeat;
        }
    }
}

void decompress (std::istream & in, std::ostream & out)
{
    char buff[240 * sizeof(uint16_t) * 5];
    size_t pos = 0;
    
    while (in)
    {
        lz4_header cur;
        in.read(reinterpret_cast <char *> (&cur), sizeof(cur));
        if (!in)
            break;
        
        in.read(buff + pos, cur.size);
        for (size_t i = 1; i != cur.repeat; ++i)
        {
            for (size_t j = 0; j != cur.size; ++j)
                buff[pos + i * cur.size + j] = buff[pos + j];
        }
        
        pos += cur.size * cur.repeat;
        if (pos == std::extent_v <decltype(buff)>)
        {
            out.write(buff, pos);
            pos = 0;
        }
    }

    if (pos != 0)
        out.write(buff, pos);
}

int main (int argc, char ** argv)
{
    if (argc < 3)
    {
        std::cout << argv[0] << " <src.rgb565> <dst.ch565>\n";
        return 1;
    }
    
    std::ifstream source(argv[1]);
    std::ofstream compressed(argv[2]);
    compress(source, compressed);
}

