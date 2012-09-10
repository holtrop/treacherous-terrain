
#include "Map.h"
#include "HexTile.h"

using namespace std;

Map::Map(int width, int height)
{
    m_width = width;
    m_height = height;
    m_grid = vector< vector< bool > >(height, vector<bool>(width));

    /* construction of default map - temporary */
    HexTile ht(width / 2.0, height / 2.0, 0.8 * height / 2.0);
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            m_grid[i][j] = ht.point_within(j + 0.5, i + 0.5);
}

bool Map::tile_present(int x, int y)
{
    return m_grid[y][x];
}
