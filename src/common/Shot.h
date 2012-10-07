
#ifndef SHOT_H
#define SHOT_H

#include <SFML/System.hpp>

class Shot
{
    public:
        Shot(const sf::Vector2f & origin, double direction, double target_dist);
        sf::Vector3f get_position();
    protected:
        sf::Vector2f m_origin;
        sf::Vector2f m_direction;
        double m_speed;
        sf::Clock m_clock;
};

#endif