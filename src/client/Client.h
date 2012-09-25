
#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Window.hpp>
#include <SFML/Graphics/Texture.hpp>
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
        void run(bool fullscreen, int width, int height, std::string pname);
    protected:
        bool create_window(bool fullscreen, int width, int height);
        bool initgl();
        void resize_window(int width, int height);
        void update(double elapsed_time);
        void redraw();
        void draw_player(Player player);
        void draw_map();
        void draw_overlay();
        void draw_sky();
        void draw_lava();

        refptr<sf::Window> m_window;
        sf::Clock m_clock;
        Map m_map;
		sf::Uint8 current_player;
		std::string current_player_name;
        refptr< std::map<sf::Uint8, Player> > m_players;
        int m_width;
        int m_height;
        GLProgram m_obj_program;
        GLProgram m_overlay_program;
        GLProgram m_sky_program;
        GLProgram m_lava_program;
        WFObj m_tank_obj;
        WFObj m_tile_obj;
        GLMatrix m_projection;
        GLMatrix m_modelview;
        GLBuffer m_overlay_hex_attributes;
        GLBuffer m_overlay_hex_indices;
        GLBuffer m_sky_attributes;
        GLBuffer m_tex_quad_attributes;
        refptr<Network> m_net_client;
        bool client_has_focus;
        sf::Texture m_lava_texture;
};

#endif
