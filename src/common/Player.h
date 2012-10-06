#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "Types.h"
#include "SFML/Config.hpp"
#include "Network.h"

class Player
{
    public:
        std::string name;
        double x;
        double y;
        double direction; /* 0 = East, M_PI_2 = North, M_PI = West, ... */
        double hover;
        sf::Uint8 w_pressed;
        sf::Uint8 a_pressed;
        sf::Uint8 s_pressed;
        sf::Uint8 d_pressed;
        sf::Int32 rel_mouse_movement;
        bool updated;
        Client_t* m_client;
        float m_shot_distance;
        double m_shot_start_x;
        double m_shot_start_y;
        double m_shot_direction;
        double m_shot_start_time;
        bool m_shot_allowed;

        Player();
};

#endif
