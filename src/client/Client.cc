
#include <math.h>
#include "Client.h"
#include "Types.h"

Client::Client()
{
    m_net_client = new Network();
    m_net_client->Create(59243, "127.0.0.1"); // Just connect to local host for now - testing
    client_has_focus = true;
}

Client::~Client()
{
    m_net_client->Destroy();
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
            case sf::Event::LostFocus:
                client_has_focus = false;
                break;
            case sf::Event::GainedFocus:
                client_has_focus = true;
                break;
            default:
                break;
            }
        }

        update(elapsed_time);
        redraw();
        last_time = current_time;

        // temporary for now.  otherwise this thread consumed way too processing
        sf::sleep(sf::seconds(0.005)); // 5 milli-seconds
    }
}

void Client::update(double elapsed_time)
{
    static sf::Uint8 w_pressed_prev = KEY_NOT_PRESSED;
    static sf::Uint8 a_pressed_prev = KEY_NOT_PRESSED;
    static sf::Uint8 s_pressed_prev = KEY_NOT_PRESSED;
    static sf::Uint8 d_pressed_prev = KEY_NOT_PRESSED;
    static sf::Int32 rel_mouse_movement_prev = 0;

    sf::Packet client_packet;

    m_net_client->Receive();
    client_packet.clear();
	// Handle all received data (only really want the latest)
    while(m_net_client->getData(client_packet))
    {
        // Update player position as calculated from the server.
        client_packet >> m_player->direction;
        client_packet >> m_player->x;
        client_packet >> m_player->y;
    }

    // For now, we are going to do a very crude shove data into
    // packet from keyboard and mouse events.
    // TODO:  Clean this up and make it more robust
    client_packet.clear();

    sf::Uint8 w_pressed = KEY_NOT_PRESSED;
    sf::Uint8 a_pressed = KEY_NOT_PRESSED;
    sf::Uint8 s_pressed = KEY_NOT_PRESSED;
    sf::Uint8 d_pressed = KEY_NOT_PRESSED;
    sf::Int32 rel_mouse_movement = 0;

	// This is a fix so that the mouse will not move outside the window and
    // cause the user to click on another program.
    // Note:  Does not work well with fast movement.
    if(client_has_focus)
    {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			a_pressed = KEY_PRESSED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			d_pressed = KEY_PRESSED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			w_pressed = KEY_PRESSED;
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
		{
			s_pressed = KEY_PRESSED;
		}
		rel_mouse_movement = sf::Mouse::getPosition(*m_window).x - m_width / 2;
        sf::Mouse::setPosition(sf::Vector2i(m_width / 2, m_height / 2), *m_window);
    }

    // Send an update to the server if something has changed
    if((w_pressed_prev != w_pressed) ||
       (a_pressed_prev != a_pressed) ||
       (s_pressed_prev != s_pressed) ||
       (d_pressed_prev != d_pressed) ||
       (rel_mouse_movement_prev !=  rel_mouse_movement))
    {
        client_packet << w_pressed;
        client_packet << a_pressed;
        client_packet << s_pressed;
        client_packet << d_pressed;
        client_packet << rel_mouse_movement;

        m_net_client->sendData(client_packet);

        w_pressed_prev = w_pressed;
        a_pressed_prev = a_pressed;
        s_pressed_prev = s_pressed;
        d_pressed_prev = d_pressed;
        rel_mouse_movement_prev =  rel_mouse_movement;
    }
    m_net_client->Transmit();
}
