
#include <getopt.h>
#include <SFML/Window.hpp>

int main(int argc, char *argv[])
{
    bool fullscreen = false;
    for (;;)
    {
        int c = getopt_long(argc, argv, "f", NULL, NULL);
        if (c == -1)
            break;
        switch (c)
        {
            case 'f':
                fullscreen = true;
                break;
        }
    }
    sf::VideoMode mode = fullscreen
        ? sf::VideoMode::getDesktopMode()
        : sf::VideoMode(800, 600, 32);
    long style = fullscreen
        ? sf::Style::Fullscreen
        : sf::Style::Resize | sf::Style::Close;
    sf::Window window(mode, "Treacherous Terrain", style);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if ( (event.type == sf::Event::KeyPressed)
                    && (event.key.code == sf::Keyboard::Escape) )
                window.close();
        }

        window.display();
    }

    return 0;
}
