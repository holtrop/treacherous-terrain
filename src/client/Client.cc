
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
        ? sf::VideoMode::GetDesktopMode()
        : sf::VideoMode(800, 600, 32);
    long style = fullscreen
        ? sf::Style::Fullscreen
        : sf::Style::Resize | sf::Style::Close;
    sf::Window window(mode, "Treacherous Terrain", style);

    while (window.IsOpened())
    {
        sf::Event event;
        while (window.GetEvent(event))
        {
            if (event.Type == sf::Event::Closed)
                window.Close();

            if ( (event.Type == sf::Event::KeyPressed)
                    && (event.Key.Code == sf::Key::Escape) )
                window.Close();
        }

        window.Display();
    }

    return 0;
}
