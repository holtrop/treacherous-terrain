#include "Map.h"
#include "HexTile.h"

using namespace std;

Map::Map(int width, int height, float tile_size)
{
    m_width = width;
    m_height = height;
    m_tile_size = tile_size;

    /* construction of default map */
    float span_x = HEX_WIDTH_TO_HEIGHT * 0.75 * tile_size;
    float offset_x = -span_x * (width / 2.0);
    float offset_y = -tile_size * (height / 2.0);

    HexTile outer_out(0, 0, 0.85 * span_x * width);
    HexTile outer_in(0, 0, 0.5 * span_x * width);
    HexTile inner_out(0, 0, 0.30 * span_x * width);

    for (int i = 0; i < height; i++)
    {
        m_grid.push_back(vector< refptr< HexTile > >());
        for (int j = 0; j < width; j++)
        {
            refptr<HexTile> ht;
            float x = j * span_x + offset_x;
            float y = (i + ((j & 1) ? 0.5 : 0.0)) * tile_size + offset_y;

            if (inner_out.point_within(y, x)
                    || (outer_out.point_within(y, x)
                        && !outer_in.point_within(y, x)))
            {
                ht = new HexTile(x, y, tile_size);
            }

            m_grid[i].push_back(ht);
        }
    }
}
