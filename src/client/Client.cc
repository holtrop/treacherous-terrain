
#include "Client.h"

Client::Client(bool fullscreen)
{
    sf::VideoMode mode = fullscreen
        ? sf::VideoMode::getDesktopMode()
        : sf::VideoMode(800, 600, 32);
    long style = fullscreen
        ? sf::Style::Fullscreen
        : sf::Style::Resize | sf::Style::Close;
    m_window = new sf::Window(mode, "Treacherous Terrain", style);
}

void Client::run()
{
    while (m_window->isOpen())
    {
        sf::Event event;
        while (m_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                m_window->close();

            if ( (event.type == sf::Event::KeyPressed)
                    && (event.key.code == sf::Keyboard::Escape) )
                m_window->close();
        }

        m_window->display();
    }
}
