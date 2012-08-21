
#ifndef MAP_H
#define MAP_H

#include <vector>

class Map
{
    public:
        Map(int width=50, int height=50);
        bool tile_present(int x, int y);
        int get_width() { return m_width; }
        int get_height() { return m_height; }
    protected:
        int m_width;
        int m_height;
        std::vector< std::vector< bool > > m_grid;
};

#endif
