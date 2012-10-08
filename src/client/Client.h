#ifndef CLIENT_H
#define CLIENT_H

#include <map>
#include <list>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Texture.hpp>
#include "refptr.h"
#include "Map.h"
#include "Shot.h"
#include "Player.h"
#include "GLProgram.h"
#include "WFObj.h"
#include "GLMatrix.h"
#include "GLBuffer.h"
#include "Network.h"

#define SHOT_RING_WIDTH 10.0f

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
        void grab_mouse(bool grab);
        void recenter_cursor();
        void draw_player(refptr<Player> player);
        void draw_map();
        void draw_shot(refptr<Shot> shot);
        void draw_overlay();
        void draw_sky();
        void draw_lava();
        void draw_shot_ring();
        void draw_shot_ring_instance();
        void create_shot();

        bool m_mouse_grabbed;
        double m_player_dir_x;
        double m_player_dir_y;
        refptr<sf::Window> m_window;
        sf::Clock m_clock;
        Map m_map;
        sf::Uint8 m_current_player;
        std::string m_current_player_name;
        std::map<sf::Uint8, refptr<Player> > m_players;
        int m_width;
        int m_height;
        GLProgram m_obj_program;
        GLProgram m_overlay_program;
        GLProgram m_overlay_hover_program;
        GLProgram m_sky_program;
        GLProgram m_lava_program;
        GLProgram m_shot_ring_program;
        WFObj m_tank_obj;
        WFObj m_tile_obj;
        WFObj m_tile_damaged_obj;
        GLMatrix m_projection;
        GLMatrix m_modelview;
        GLBuffer m_overlay_hex_attributes;
        GLBuffer m_overlay_hex_indices;
        GLBuffer m_sky_attributes;
        GLBuffer m_tex_quad_attributes;
        GLBuffer m_quad_attributes;
        GLBuffer m_shot_ring_attributes;
        GLBuffer m_sphere_attributes;
        GLBuffer m_sphere_indices;
        refptr<Network> m_net_client;
        bool m_client_has_focus;
        sf::Texture m_lava_texture;
        bool m_left_button_pressed;
        bool m_drawing_shot;
        float m_drawing_shot_distance;
        bool m_shot_fired;
};

#endif
