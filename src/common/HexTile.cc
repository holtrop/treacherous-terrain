#include <math.h>
#include "HexTile.h"

#define COS_60 0.5f
#define SIN_60 0.8660254037844386f
#define COS_120 (-0.5f)
#define SIN_120 SIN_60

HexTile::HexTile(float x, float y, float size)
{
    m_x = x;
    m_y = y;
    m_size = size;
    m_damage_state = UNDAMAGED;
}

bool HexTile::point_within(float x, float y)
{
    x = 2.0 * (x - m_x);
    y = 2.0 * (y - m_y);
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
    if (fabsf(y) > m_size)
        return false;
    float y_60 = y * COS_60 + x * SIN_60;
    if (fabsf(y_60) > m_size)
        return false;
    float y_120 = y * COS_120 + x * SIN_120;
    if (fabsf(y_120) > m_size)
        return false;
    return true;
}

void HexTile::shot()
{
    if(m_damage_state < DESTROYED)
    {
        // Note:  This only works because these are in order in the ENUM.
        m_damage_state = (Tile_Damage_States_t)((int)(m_damage_state) + 1);
    }
}
