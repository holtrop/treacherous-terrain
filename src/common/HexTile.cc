
#include "HexTile.h"

#define COS_60 0.5
#define SIN_60 0.8660254037844386

/* points of a horizontal hexagon 2.0 units high */
static const float hex_points[][2] = {
    {1.1547005383792517, 0.0},
    {0.577350269189626, 1.0},
    {-0.5773502691896256, 1.0},
    {-1.1547005383792517, 0.0},
    {-0.5773502691896264, -1.0},
    {0.577350269189626, -1.0}
};

HexTile::HexTile(float x, float y, float size)
{
    m_x = x;
    m_y = y;
    m_size = size;
}

bool HexTile::point_within(float x, float y)
{
    x = 2.0 * (x - m_x) / m_size;
    y = 2.0 * (y - m_y) / m_size;
    /* a point (x, y) at angle q rotates to (x', y') by f deg around the origin
     * according to:
     *  x' = r*cos(q+f) = r*cos(q)*cos(f)-r*sin(q)*sin(f)
     *  y' = r*sin(q+f) = r*sin(q)*cos(f)+r*cos(q)*sin(f)
     *  x' = x*cos(f) - y*sin(f)
     *  y' = y*cos(f) + x*sin(f)
     * cos(60 deg) = 0.5
     * sin(60 deg) = 0.8660254
     * so:
     *  x' = x * 0.5 - y * 0.8660254
     *  y' = y * 0.5 + x * 0.8660254
     */
    if ((y > 1.0) || (y < -1.0))
        return false;
    float rx = x * COS_60 - y * SIN_60;
    float ry = y * COS_60 + x * SIN_60;
    if ((ry > 1.0) || (ry < -1.0))
        return false;
    x = rx * COS_60 - ry * SIN_60;
    y = ry * COS_60 + rx * SIN_60;
    if ((y > 1.0) || (y < -1.0))
        return false;
    return true;
}
