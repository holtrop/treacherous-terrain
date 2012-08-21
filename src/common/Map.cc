
#include "Map.h"

using namespace std;

Map::Map(int width, int height)
{
    m_width = width;
    m_height = height;
    m_grid = vector< vector< bool > >(height, vector<bool>(width, true));
}

bool Map::tile_present(int x, int y)
{
    return m_grid[y][x];
}
