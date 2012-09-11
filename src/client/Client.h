
#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Window.hpp>
#include "refptr.h"
#include "Map.h"
#include "Player.h"
#include "GLProgram.h"
#include "WFObj.h"
#include "GLMatrix.h"

class Client
{
    public:
        Client(bool fullscreen, bool compatibility_context,
                unsigned int antialias_level);
        void run();
    protected:
        void initgl();
        void resize_window(int width, int height);
        void update(double elapsed_time);
        void draw_players();
        void draw_map();
        refptr<sf::Window> m_window;
        sf::Clock m_clock;
        Map m_map;
        refptr<Player> m_player;
        int m_width;
        int m_height;
        GLProgram m_obj_program;
        WFObj m_tank_obj;
        GLMatrix m_projection;
        GLMatrix m_modelview;
};

#endif
