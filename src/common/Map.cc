#include "ccfs.h"
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
Map::Map(const string & mapname, float tile_size)
{
    unsigned int length = 0u;
    const unsigned char *mapdata = CFS.get_file(("maps/" + mapname).c_str(), &length);

    m_width = m_height = 0;
    m_tile_size = tile_size;

    int line_width = 0;
    /* determine the map size */
    for (unsigned int i = 0; i < length; i++)
    {
        if (mapdata[i] == '\n')
        {
            if (line_width > m_width)
            {
                m_width = line_width;
            }
            line_width = 0;
            m_height++;
        }
        else
        {
            line_width++;
        }
    }

    m_span_x = HEX_WIDTH_TO_HEIGHT * 0.75 * tile_size;
    m_offset_x = -m_span_x * (m_width / 2.0);
    m_offset_y = -tile_size * (m_height / 2.0);

    for (int i = 0; i < m_height; i++)
    {
        m_grid.push_back(vector< refptr< HexTile > >());
        for (int j = 0; j < m_width; j++)
        {
            m_grid[i].push_back(refptr<HexTile>());
        }
    }

    for (unsigned int pos = 0, i = 0, j = 0; pos < length; pos++)
    {
        if (mapdata[pos] == '\n')
        {
            i++;
            j = 0;
        }
        else
        {
            if (mapdata[pos] == 'X')
            {
                float x = j * m_span_x + m_offset_x;
                float y = (i + ((j & 1) ? 0.5 : 0.0)) * tile_size + m_offset_y;

                m_grid[i][j] = new HexTile(x, y, tile_size);
            }
            j++;
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
