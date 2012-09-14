
#include <math.h>
#include "Client.h"

Client::Client()
{
}

void Client::run(bool fullscreen, int width, int height)
{
    if (!create_window(fullscreen, width, height))
        return;
    m_player = new Player();
    m_player->x = 0;
    m_player->y = 0;
    m_player->direction = M_PI_2;
    m_clock.restart();
    sf::Mouse::setPosition(sf::Vector2i(m_width / 2, m_height / 2), *m_window);
    double last_time = 0.0;
    while (m_window->isOpen())
    {
        double current_time = m_clock.getElapsedTime().asSeconds();
        double elapsed_time = current_time - last_time;
        sf::Event event;
        while (m_window->pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                m_window->close();
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    m_window->close();
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::Resized:
                resize_window(event.size.width, event.size.height);
                break;
            default:
                break;
            }
        }

        update(elapsed_time);
        redraw();
        last_time = current_time;
    }
}

void Client::update(double elapsed_time)
{
    const double move_speed = 75.0;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        double direction = m_player->direction + M_PI_2;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        double direction = m_player->direction - M_PI_2;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        double direction = m_player->direction;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        double direction = m_player->direction + M_PI;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    int xrel = sf::Mouse::getPosition(*m_window).x - m_width / 2;
    sf::Mouse::setPosition(sf::Vector2i(m_width / 2, m_height / 2), *m_window);
    m_player->direction -= M_PI * 0.5 * xrel / 1000;
}
