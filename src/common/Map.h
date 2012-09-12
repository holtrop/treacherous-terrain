
#ifndef MAP_H
#define MAP_H

#include <vector>
#include "refptr.h"
#include "HexTile.h"

class Map
{
    public:
        Map(int width=21, int height=21, float tile_size=50);
        bool tile_present(int x, int y) { return !m_grid[y][x].isNull(); }
        refptr<HexTile> get_tile(int x, int y) { return m_grid[y][x]; }
        int get_width() { return m_width; }
        int get_height() { return m_height; }
    protected:
        int m_width;
        int m_height;
        float m_tile_size;
        std::vector< std::vector< refptr< HexTile > > > m_grid;
};

#endif
