
#ifndef HEXTILE_H
#define HEXTILE_H

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
