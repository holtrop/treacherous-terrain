
#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Window.hpp>
#include "refptr.h"

class Client
{
    public:
        Client(bool fullscreen);
        void run();
    protected:
        void initgl();
        void resize_window(int width, int height);
        refptr<sf::Window> m_window;
        sf::Clock m_clock;
};

#endif
