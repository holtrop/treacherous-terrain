
#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Window.hpp>
#include "refptr.h"
#include "Map.h"

class Client
{
    public:
        Client(bool fullscreen);
        void run();
    protected:
        void initgl();
        void resize_window(int width, int height);
        void draw_map();
        refptr<sf::Window> m_window;
        sf::Clock m_clock;
        Map m_map;
};

#endif
