
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
        refptr<sf::Window> m_window;
};

#endif
