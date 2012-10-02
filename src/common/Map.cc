#include "Map.h"
#include "HexTile.h"

using namespace std;

/*
 * The center of the tile at m_grid[i][j] is (x, y) where
 * x = j * m_span_x + m_offset_x
 * y = (i + ((j & 1) ? 0.5 : 0.0) * m_tile_size + m_offset_y
 *
 * The rectangular bounding box for a hex tile centered at (x, y)
 * are ((x - HEX_WIDTH_TO_HEIGHT * m_tile_size / 2, y - m_tile_size / 2),
 *      (x + HEX_WIDTH_TO_HEIGHT * m_tile_size / 2, y + m_tile_size / 2))
 */
Map::Map(int width, int height, float tile_size)
{
    m_width = width;
    m_height = height;
    m_tile_size = tile_size;

    /* construction of default map */
    m_span_x = HEX_WIDTH_TO_HEIGHT * 0.75 * tile_size;
    m_offset_x = -m_span_x * (width / 2.0);
    m_offset_y = -tile_size * (height / 2.0);

    HexTile outer_out(0, 0, 0.85 * m_span_x * width);
    HexTile outer_in(0, 0, 0.5 * m_span_x * width);
    HexTile inner_out(0, 0, 0.30 * m_span_x * width);

    for (int i = 0; i < height; i++)
    {
        m_grid.push_back(vector< refptr< HexTile > >());
        for (int j = 0; j < width; j++)
        {
            refptr<HexTile> ht;
            float x = j * m_span_x + m_offset_x;
            float y = (i + ((j & 1) ? 0.5 : 0.0)) * tile_size + m_offset_y;

            if (inner_out.point_within(y, x)
                    || (outer_out.point_within(y, x)
                        && !outer_in.point_within(y, x)))
            {
                refptr<HexTile> get_tile_at(float x, float y);
                ht = new HexTile(x, y, tile_size);
            }

            m_grid[i].push_back(ht);
        }
    }
}

/*
 * Given a point (x, y) return the HexTile, if any, that the point is on.
 * A NULL refptr is returned if the point is not on any tile.
 */
refptr<HexTile> Map::get_tile_at(float x, float y)
{
    int i_base = (int) ((y - m_offset_y) / m_tile_size);
    int j_base = (int) ((x - m_offset_x) / m_span_x);
    for (int i_offset = 0; i_offset <= 1; i_offset++)
    {
        for (int j_offset = 0; j_offset <= 1; j_offset++)
        {
            int i = i_base + i_offset;
            int j = j_base + j_offset;
            if (i >= 0 && i < m_height && j >= 0 && j < m_width)
            {
                if (!m_grid[i][j].isNull())
                    if (m_grid[i][j]->point_within(x, y))
                        return m_grid[i][j];
            }
        }
    }
    return NULL;
}
