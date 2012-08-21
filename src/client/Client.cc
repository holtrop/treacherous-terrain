
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

        draw_map();

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
    gluPerspective(60.0f, aspect, 0.01, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-70, 1, 0, 0);
}

void Client::draw_map()
{
    const int width = m_map.get_width();
    const int height = m_map.get_height();
    const float span_x = 50;
    const float span_y = 50;
    float center_x = (span_x * width) / 2.0;
    glPushAttrib(GL_POLYGON_BIT);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPushMatrix();
    glTranslatef(-center_x, 0, -100);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            glPushMatrix();
            glTranslatef(span_x * x, span_y * y, 0);
            glBegin(GL_QUADS);
            glColor3f(0.4, 0.4, 0.4);
            glVertex2f(span_x, span_y);
            glVertex2f(0, span_y);
            glVertex2f(0, 0);
            glVertex2f(span_x, 0);
            glEnd();
            glBegin(GL_LINE_LOOP);
            glColor3f(1, 1, 1);
            glVertex2f(span_x, span_y);
            glVertex2f(0, span_y);
            glVertex2f(0, 0);
            glVertex2f(span_x, 0);
            glEnd();
            glPopMatrix();
        }
    }
    glPopMatrix();
    glPopAttrib();
}
