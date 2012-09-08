
#include <math.h>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "Client.h"
#include <stdio.h>
#include <iostream>
#include "ccfs.h"

using namespace std;

Client::Client(bool fullscreen, bool compatibility_context,
                unsigned int antialias_level)
{
    sf::VideoMode mode = fullscreen
        ? sf::VideoMode::getDesktopMode()
        : sf::VideoMode(800, 600, 32);
    long style = fullscreen
        ? sf::Style::Fullscreen
        : sf::Style::Resize | sf::Style::Close;
    const unsigned int opengl_major = compatibility_context ? 2 : 4;
    const unsigned int opengl_minor = 0u;
    sf::ContextSettings cs = sf::ContextSettings(0, 0, antialias_level,
            opengl_major, opengl_minor);
    m_window = new sf::Window(mode, "Treacherous Terrain", style, cs);
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        cerr << "GLEW Initialization error: " << glewGetErrorString(err) << endl;
    }
    initgl();
    resize_window(m_window->getSize().x, m_window->getSize().y);
    m_player = new Player();
    m_player->x = 1250;
    m_player->y = 1000;
    m_player->direction = M_PI_2;
    GLProgram::AttributeBinding obj_attrib_bindings[] = {
        {0, "pos"},
        {1, "normal"},
        {0, NULL}
    };
    const char *v_source = (const char *) CFS.get_file("shaders/obj_v.glsl", NULL);
    const char *f_source = (const char *) CFS.get_file("shaders/obj_f.glsl", NULL);
    if (v_source == NULL || f_source == NULL)
    {
        cerr << "Error loading shader sources" << endl;
    }
    else if (!m_obj_program.create(v_source, f_source,
                obj_attrib_bindings))
    {
        cerr << "Error creating obj program" << endl;
    }
}

void Client::run()
{
    m_clock.restart();
    m_window->setMouseCursorVisible(false);
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glPushMatrix();
        double dir_x = cos(m_player->direction);
        double dir_y = sin(m_player->direction);
        gluLookAt(m_player->x - dir_x * 100, m_player->y - dir_y * 100, 150,
                m_player->x, m_player->y, 100,
                0, 0, 1);

        draw_players();
        draw_map();

        glPopMatrix();

        m_window->display();
        last_time = current_time;
    }
}

void Client::initgl()
{
    glShadeModel(GL_SMOOTH);
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glPolygonOffset(0, -2);
}

void Client::resize_window(int width, int height)
{
    m_width = width;
    m_height = height;
    sf::Mouse::setPosition(sf::Vector2i(m_width / 2, m_height / 2), *m_window);
    glViewport(0, 0, width, height);
    float aspect = (float)width / (float)height;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, aspect, 0.01, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Client::update(double elapsed_time)
{
    const double move_speed = 300.0;
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

void Client::draw_players()
{
    static const float vertices[][3] = {
        {1, 1, 1},
        {-1, 1, 1},
        {-1, -1, 1},
        {1, -1, 1},
        {1, 1, -1},
        {-1, 1, -1},
        {-1, -1, -1},
        {1, -1, -1}
    };
    static const int quads[][4] = {
        {0, 1, 2, 3},
        {0, 3, 7, 4},
        {2, 1, 5, 6},
        {3, 2, 6, 7},
        {1, 0, 4, 5},
        {5, 4, 7, 6}
    };
    glPushMatrix();
    glTranslatef(m_player->x, m_player->y, 40);
    glRotatef(m_player->direction * 180.0 / M_PI, 0, 0, 1);
    glPushAttrib(GL_POLYGON_BIT);
    glEnable(GL_POLYGON_OFFSET_LINE);
    for (int t = 0; t <= 1; t++)
    {
        if (t == 0)
        {
            glColor3f(0.8, 0, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else
        {
            glColor3f(0, 0, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        for (unsigned int i = 0; i < sizeof(quads) / sizeof(quads[0]); i++)
        {
            glBegin(GL_QUADS);
            for (int j = 0; j < 4; j++)
            {
                const float * vertex = &vertices[quads[i][j]][0];
                glVertex3f(vertex[0] * 20, vertex[1] * 10, vertex[2] * 6);
            }
            glEnd();
        }
    }
    glPopAttrib();
    glPopMatrix();
}

void Client::draw_map()
{
    const int width = m_map.get_width();
    const int height = m_map.get_height();
    const float span_x = 50;
    const float span_y = 50;
    glPushAttrib(GL_POLYGON_BIT);
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPushMatrix();
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            glPushMatrix();
            glTranslatef(span_x * x, span_y * y, 0);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBegin(GL_QUADS);
            glColor3f(0.4, 0.4, 0.4);
            glVertex2f(span_x, span_y);
            glVertex2f(0, span_y);
            glVertex2f(0, 0);
            glVertex2f(span_x, 0);
            glEnd();
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBegin(GL_QUADS);
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
