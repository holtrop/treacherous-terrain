
#ifndef HEXTILE_H
#define HEXTILE_H

#define HEX_WIDTH_TO_HEIGHT 1.1547005383792517

class HexTile
{
    public:
        HexTile(float x, float y, float size);
        bool point_within(float x, float y);
    protected:
        float m_x;
        float m_y;
        float m_size;
};

#endif
