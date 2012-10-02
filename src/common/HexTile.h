#ifndef HEXTILE_H
#define HEXTILE_H

#define HEX_WIDTH_TO_HEIGHT 1.1547005383792517

class HexTile
{
    public:
        typedef enum{
            UNDAMAGED,
            DAMAGED,
            DESTROYED
        }Tile_Damage_States_t;

        HexTile(float x, float y, float size);
        bool point_within(float x, float y);
        float get_x() { return m_x; }
        float get_y() { return m_y; }
        float get_size() { return m_size; }
        Tile_Damage_States_t get_damage_state() { return m_damage_state; }
        void shot();
    protected:
        float m_x;
        float m_y;
        float m_size;
        Tile_Damage_States_t m_damage_state;
        
};

#endif
