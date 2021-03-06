
#ifndef SHOT_H
#define SHOT_H

#include <SFML/System.hpp>

class Shot
{
    public:
        Shot(const sf::Vector2f & origin, double direction, double target_dist);
        sf::Vector3f get_position();
        double get_elapsed_time()
        {
            return m_clock.getElapsedTime().asSeconds();
        }
        double get_duration() { return m_duration; }
    protected:
        sf::Vector2f m_origin;
        sf::Vector2f m_direction;
        double m_speed;
        double m_cos_a;
        double m_sin_a;
        sf::Clock m_clock;
        double m_duration;
};

#endif
