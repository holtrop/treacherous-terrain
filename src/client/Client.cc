#include <math.h>
#include "Client.h"
#include "Types.h"
#include "Timer.h"

/* TODO: this should be moved to common somewhere */
#define MAX_SHOT_DISTANCE 250.0
#define SHOT_EXPAND_SPEED 75.0

Client::Client()
{
    m_net_client = new Network();
    m_net_client->Create(59243, "127.0.0.1"); // Just connect to local host for now - testing
    client_has_focus = true;
    m_players.clear();
    current_player = 0;
    m_left_button_pressed = false;
    m_drawing_shot = false;
}

Client::~Client()
{
    m_net_client->Destroy();
}


void Client::run(bool fullscreen, int width, int height, std::string pname)
{
    Timer client_timer;
    client_timer.Init();
    current_player_name = pname;
    if (!create_window(fullscreen, width, height))
        return;
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
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left)
                    m_left_button_pressed = true;
                break;
            case sf::Event::MouseButtonReleased:
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    m_drawing_shot = false;
                    m_left_button_pressed = false;
                    /* TODO: trigger shot network message */
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

        // Time must be updated before any messages are sent
        // Especially guaranteed messages, since the time needs to be
        // non zero.
        client_timer.Update();

        update(elapsed_time);
        redraw();
        last_time = current_time;

        // temporary for now.  otherwise this thread consumed way too processing
        sf::sleep(sf::seconds(0.005)); // 5 milli-seconds
    }
}

void Client::update(double elapsed_time)
{
    static bool registered_player = false;
    sf::Packet client_packet;

    m_net_client->Receive();
    client_packet.clear();
    // Handle all received data (only really want the latest)
    while(m_net_client->getData(client_packet))
    {
        sf::Uint8 packet_type;
        client_packet >> packet_type;
        switch(packet_type)
        {
            case PLAYER_CONNECT:
            {
                sf::Uint8 pindex;
                std::string name = "";
                client_packet >> pindex;
                client_packet >> name;
                // Should be a much better way of doing this.
                // Perhaps generate a random number
                if(name == current_player_name)
                {
                    current_player = pindex;
                }

                // Create a new player if one does not exist.
                if(m_players.end() == m_players.find(pindex))
                {
                    refptr<Player> p = new Player();
                    p->name = name;
                    client_packet >> p->direction;
                    client_packet >> p->x;
                    client_packet >> p->y;
                    m_players[pindex] = p;
                }
                break;
            }
            case PLAYER_UPDATE:
            {
                sf::Uint8 player_index;
                // Update player position as calculated from the server.
                client_packet >> player_index;
                if(m_players.end() != m_players.find(player_index))
                {
                    client_packet >> m_players[player_index]->direction;
                    client_packet >> m_players[player_index]->x;
                    client_packet >> m_players[player_index]->y;
                }
                break;
            }
            case PLAYER_DISCONNECT:
            {
                // This will remove the player once the disconnect algorithm is implemented
                break;
            }
            case PLAYER_DEATH:
            {
                // This will set a death flag in the player struct.
                break;
            }
            default :
            {
                // Eat the packet
                break;
            }
        }
    }

    // For now, we are going to do a very crude shove data into
    // packet from keyboard and mouse events.
    // TODO:  Clean this up and make it more robust
    if(m_players.size() > 0)
    {
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

            if (m_left_button_pressed)
            {
                if (m_drawing_shot)
                {
                    m_drawing_shot_distance += SHOT_EXPAND_SPEED * elapsed_time;
                    if (m_drawing_shot_distance > MAX_SHOT_DISTANCE)
                        m_drawing_shot_distance = MAX_SHOT_DISTANCE;
                }
                else
                {
                    m_drawing_shot = true;
                    m_drawing_shot_distance = 0.0f;
                }
            }
        }

        /* decrease player hover when not over a tile */
        if (m_map.get_tile_at(m_players[current_player]->x,
                    m_players[current_player]->y).isNull())
        {
            m_players[current_player]->hover -= elapsed_time / 10;
            if (m_players[current_player]->hover < 0)
                m_players[current_player]->hover = 0;
        }

        m_player_dir_x = cos(m_players[current_player]->direction);
        m_player_dir_y = sin(m_players[current_player]->direction);

        // Send an update to the server if something has changed
        if((m_players[current_player]->w_pressed != w_pressed) ||
           (m_players[current_player]->a_pressed != a_pressed) ||
           (m_players[current_player]->s_pressed != s_pressed) ||
           (m_players[current_player]->d_pressed != d_pressed) ||
           (m_players[current_player]->rel_mouse_movement !=  rel_mouse_movement))
        {
            sf::Uint8 packet_type = PLAYER_UPDATE;
            client_packet.clear();
            client_packet << packet_type;
            client_packet << current_player;
            client_packet << w_pressed;
            client_packet << a_pressed;
            client_packet << s_pressed;
            client_packet << d_pressed;
            client_packet << rel_mouse_movement;

            m_net_client->sendData(client_packet);

            m_players[current_player]->w_pressed = w_pressed;
            m_players[current_player]->a_pressed = a_pressed;
            m_players[current_player]->s_pressed = s_pressed;
            m_players[current_player]->d_pressed = d_pressed;
            m_players[current_player]->rel_mouse_movement =  rel_mouse_movement;
        }
    }
    else if(!registered_player)
    {
        sf::Uint8 packet_type = PLAYER_CONNECT;
        client_packet.clear();
        client_packet << packet_type;
        client_packet << current_player;
        client_packet << current_player_name;
        m_net_client->sendData(client_packet, true);
        registered_player = true;
    }
    else
    {
        // Do nothing.
    }
    m_net_client->Transmit();
}
