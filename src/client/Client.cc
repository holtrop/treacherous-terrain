
#include "Client.h"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

Client::Client(bool fullscreen)
{
    sf::VideoMode mode = fullscreen
        ? sf::VideoMode::getDesktopMode()
        : sf::VideoMode(800, 600, 32);
    long style = fullscreen
        ? sf::Style::Fullscreen
        : sf::Style::Resize | sf::Style::Close;
    m_window = new sf::Window(mode, "Treacherous Terrain", style);
    resize_window(m_window->getSize().x, m_window->getSize().y);
}

void Client::run()
{
    while (m_window->isOpen())
    {
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* simple fixed-functionality drawing demo for now */
        glPushMatrix();
        glRotatef(m_clock.getElapsedTime().asSeconds() * 180.0f, 0, 0, 1);
        glBegin(GL_QUADS);
        glColor3f(1, 0.6, 0.1);
        glVertex2f(1, 1);
        glColor3f(0, 0, 1);
        glVertex2f(-1, 1);
        glColor3f(1, 0, 0);
        glVertex2f(-1, -1);
        glColor3f(0, 1, 0);
        glVertex2f(1, -1);
        glEnd();
        glPopMatrix();

        m_window->display();
    }
}

void Client::initgl()
{
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);
}

void Client::resize_window(int width, int height)
{
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.2 * aspect, 1.2 * aspect, -1.2, 1.2, 1, -1);
    glMatrixMode(GL_MODELVIEW);
}
