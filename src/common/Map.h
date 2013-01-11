#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include "refptr.h"
#include "HexTile.h"

class Map
{
    public:
        Map(const std::string & mapname = "default", float tile_size = 50);
        bool tile_present(int x, int y) { return !m_grid[y][x].isNull(); }
        refptr<HexTile> get_tile(int x, int y) { return m_grid[y][x]; }
        refptr<HexTile> get_tile_at(float x, float y);
        int get_width() { return m_width; }
        int get_height() { return m_height; }
    protected:
        int m_width;
        int m_height;
        float m_tile_size;
        float m_span_x;
        float m_offset_x;
        float m_offset_y;
        std::vector< std::vector< refptr< HexTile > > > m_grid;
};

#endif
