
#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Window.hpp>
#include "refptr.h"
#include "Map.h"
#include "Player.h"
#include "GLProgram.h"
#include "WFObj.h"
#include "GLMatrix.h"
#include "GLBuffer.h"
#include "Network.h"

class Client
{
    public:
        Client();
        ~Client();
        void run(bool fullscreen, int width, int height);
    protected:
        bool create_window(bool fullscreen, int width, int height);
        bool initgl();
        void resize_window(int width, int height);
        void update(double elapsed_time);
        void redraw();
        void draw_players();
        void draw_map();
        void draw_overlay();
        void draw_sky();

        refptr<sf::Window> m_window;
        sf::Clock m_clock;
        Map m_map;
        refptr<Player> m_player;
        int m_width;
        int m_height;
        GLProgram m_obj_program;
        GLProgram m_overlay_program;
        GLProgram m_sky_program;
        WFObj m_tank_obj;
        WFObj m_tile_obj;
        GLMatrix m_projection;
        GLMatrix m_modelview;
        GLBuffer m_overlay_hex_attributes;
        GLBuffer m_overlay_hex_indices;
        GLBuffer m_sky_attributes;
        refptr<Network> m_net_client;
        bool client_has_focus;
};

#endif
