#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include "Client.h"
#include "Types.h"
#include "GameParams.h"

using namespace std;

Client::Client(const string & exe_path)
{
    m_client_has_focus = true;
    m_exe_path = exe_path;
    m_server_pid = 0;
}

Client::~Client()
{
}

void Client::connect(int port, const char *host)
{
    m_net_client = new Network();
    m_net_client->Create(port, host);
    m_players.clear();
    m_current_player = 0;
    m_left_button_pressed = false;
    m_drawing_shot = false;
    m_shot_fired = false;
    m_map = Map();
}

void Client::disconnect()
{
    // Send disconnect message
    bool connection_closed = false;
    sf::Packet client_packet;
    sf::Uint8 packet_type = PLAYER_DISCONNECT;
    sf::Clock timeout_clock;
    client_packet.clear();
    client_packet << packet_type;
    client_packet << m_current_player;
    m_net_client->sendData(client_packet, true);

    // No time out needed here, since the
    // message will timeout after a couple of attempts
    // then exit anyway.
    while(!connection_closed)
    {
        m_net_client->Receive();

        while(m_net_client->getData(client_packet))
        {
            sf::Uint8 packet_type;
            client_packet >> packet_type;
            switch(packet_type)
            {
                case PLAYER_DISCONNECT:
                {
                    sf::Uint8 player_index;
                    // This completely removes the player from the game
                    // Deletes member from the player list
                    client_packet >> player_index;
                    if(player_index == m_current_player)
                    {
                        connection_closed = true;
                    }
                    break;
                }
            }
        }

        m_net_client->Transmit();

        // temporary for now.  otherwise this thread consumed way too processing
        sf::sleep(sf::seconds(0.005)); // 5 milli-seconds

        // If the server does not respond within one second just close
        // and the server can deal with the problems.
        if(timeout_clock.getElapsedTime().asSeconds() > 1.0)
        {
            connection_closed = true;
        }
    }

    m_net_client->Destroy();
    m_net_client = NULL;
}

void Client::run(bool fullscreen, int width, int height, std::string pname)
{
    m_current_player_name = pname;
    if (!create_window(fullscreen, width, height))
        return;
    m_clock.restart();
    recenter_cursor();

    bool in_game = true;
    while (in_game)
    {
        run_main_menu();
        switch (m_menu_action)
        {
        case MAIN_MENU_SINGLE:
            start_server();
            connect(DEFAULT_PORT, "127.0.0.1");
            run_client();
            stop_server();
            disconnect();
            break;
        case MAIN_MENU_EXIT:
            in_game = false;
            break;
        }
    }
}

void Client::play_single_player_game_button_clicked()
{
    m_menu_action = MAIN_MENU_SINGLE;
}

void Client::exit_button_clicked()
{
    m_menu_action = MAIN_MENU_EXIT;
}

void Client::run_main_menu()
{
    m_window->setMouseCursorVisible(true);
    m_window->resetGLStates();

    m_menu_action = MAIN_MENU_NONE;
    sfg::Box::Ptr box = sfg::Box::Create(sfg::Box::VERTICAL, 10.0f);
    sfg::Button::Ptr btn_singleplayer =
        sfg::Button::Create("Play Single Player Game");
    btn_singleplayer->GetSignal(sfg::Widget::OnLeftClick).Connect(
            &Client::play_single_player_game_button_clicked, this);
    sfg::Button::Ptr btn_hostgame = sfg::Button::Create("Host a Network Game");
    sfg::Button::Ptr btn_joingame = sfg::Button::Create("Join a Network Game");
    sfg::Button::Ptr btn_exit = sfg::Button::Create("Exit");
    btn_exit->GetSignal(sfg::Widget::OnLeftClick).Connect(
            &Client::exit_button_clicked, this);
    box->Pack(btn_singleplayer);
    box->Pack(btn_hostgame);
    box->Pack(btn_joingame);
    box->Pack(btn_exit);
    sfg::Window::Ptr gui_window(sfg::Window::Create(sfg::Window::TITLEBAR |
                sfg::Window::BACKGROUND));
    gui_window->SetTitle("Treacherous Terrain");
    gui_window->Add(box);
    gui_window->SetPosition(sf::Vector2f(
                m_width / 2 - gui_window->GetAllocation().width / 2,
                m_height / 2 - gui_window->GetAllocation().height / 2));
    sfg::Desktop desktop;
    desktop.Add(gui_window);

    sf::Event event;

    bool in_menu = true;
    while (in_menu && m_window->isOpen())
    {
        while (m_window->pollEvent(event))
        {
            desktop.HandleEvent(event);

            switch (event.type)
            {
            case sf::Event::Closed:
                m_window->close();
                in_menu = false;
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code)
                {
                case sf::Keyboard::Escape:
                    m_menu_action = MAIN_MENU_EXIT;
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::Resized:
                m_width = event.size.width;
                m_height = event.size.height;
                gui_window->SetPosition(sf::Vector2f(
                            m_width / 2 - gui_window->GetAllocation().width / 2,
                            m_height / 2 - gui_window->GetAllocation().height / 2));
                break;
            default:
                break;
            }
        }

        desktop.Update(m_clock.restart().asSeconds());
        m_window->clear();
        m_sfgui.Display(*m_window);
        m_window->display();

        if (m_menu_action != MAIN_MENU_NONE)
            break;
    }
}

bool Client::start_server()
{
    string server_exe_path = m_exe_path;
    int length = server_exe_path.length();
    if (length > 4)
    {
        string ext = server_exe_path.substr(length - 4);
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".exe")
            server_exe_path = server_exe_path.substr(0, length - 4);
    }
    server_exe_path += "-server";
    pid_t pid = fork();
    if (pid < 0)
        return false;
    if (pid > 0)
    {
        m_server_pid = pid;
        return true;
    }
    execl(server_exe_path.c_str(), "treacherous-terrain-server", NULL);
    exit(-1);
}

void Client::stop_server()
{
    if (m_server_pid > 0)
    {
        kill(m_server_pid, SIGINT);
        m_server_pid = 0;
        waitpid(-1, NULL, 0);
    }
}

void Client::run_client()
{
    m_window->setMouseCursorVisible(false);
    double last_time = 0.0;
    bool in_game = true;
    while (in_game && m_window->isOpen())
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
                    in_game = false;
                    break;
                case sf::Keyboard::F1:
                    grab_mouse(!m_mouse_grabbed);
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::MouseButtonPressed:
                if((event.mouseButton.button == sf::Mouse::Left) &&
                   (m_shot_fired == false) && // Don't allow shots ontop of each other
                   // The server needs to allow player to shoot, so that
                   // multiple shots cannot be fired at the same time
                   (m_players[m_current_player]->m_shot_allowed) &&
                   (!m_players[m_current_player]->m_is_dead) &&
                   (m_client_has_focus))
                {
                    m_left_button_pressed = true;
                }
                break;
            case sf::Event::MouseButtonReleased:
                if((event.mouseButton.button == sf::Mouse::Left) &&
                   // Prevents a shot from being fired upon release
                   // while another shot is currently being fired.
                   (m_players[m_current_player]->m_shot_allowed) &&
                   (!m_players[m_current_player]->m_is_dead) &&
                   (m_left_button_pressed) &&
                   (m_client_has_focus))
                {
                    m_drawing_shot = false;
                    m_left_button_pressed = false;
                    m_shot_fired = true;
                    create_shot();
                }
                break;
            case sf::Event::Resized:
                resize_window(event.size.width, event.size.height);
                break;
            case sf::Event::LostFocus:
                m_client_has_focus = false;
                break;
            case sf::Event::GainedFocus:
                m_client_has_focus = true;
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

void Client::recenter_cursor()
{
    if (m_mouse_grabbed)
        sf::Mouse::setPosition(sf::Vector2i(m_width / 2, m_height / 2),
                *m_window);
}

void Client::grab_mouse(bool grab)
{
    m_mouse_grabbed = grab;
    m_window->setMouseCursorVisible(!grab);
    recenter_cursor();
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
                sf::Uint16 players_port = sf::Socket::AnyPort;
                sf::Uint8 pindex;
                std::string name = "";
                client_packet >> pindex;
                client_packet >> name;
                client_packet >> players_port;
                // Should be a much better way of doing this.
                // Perhaps generate a random number
                if((name == m_current_player_name) &&
                   (players_port == m_net_client->getLocalPort()))
                {
                    m_current_player = pindex;
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
                    client_packet >> m_players[player_index]->hover;
                }
                break;
            }
            case PLAYER_DISCONNECT:
            {
                sf::Uint8 player_index;
                // This completely removes the player from the game
                // Deletes member from the player list
                client_packet >> player_index;
                m_players.erase(player_index);
                break;
            }
            case PLAYER_DEATH:
            {
                // This will set a death flag in the player struct.
                sf::Uint8 player_index;
                // This completely removes the player from the game
                // Deletes member from the player list
                client_packet >> player_index;
                if(m_players.end() != m_players.find(player_index))
                {
                    m_players[player_index]->m_is_dead = true;
                }
                break;
            }
            
            case PLAYER_SHOT:
            {
                sf::Uint8 pindex;
                double x,y;
                double direction;
                double distance;
                
                client_packet >> pindex;
                client_packet >> x;
                client_packet >> y;
                client_packet >> direction;
                client_packet >> distance;
                
                // Ensure that the player who shot exists
                if(m_players.end() != m_players.find(pindex))
                {
                    // Perhaps sometime in the future, the shots will
                    // be different colors depending on the player
                    // or different power ups and what not.
                    refptr<Shot> shot = new Shot(sf::Vector2f(x, y),
                                                 direction,
                                                 distance);
                    m_players[pindex]->m_shot = shot;
                }
                break;
            }

            case TILE_DAMAGED:
            {
                float x;
                float y;
                sf::Uint8 pindex;
                client_packet >> x;
                client_packet >> y;
                client_packet >> pindex;
                // Damage the tile if it exists
                if((!m_map.get_tile_at(x, y).isNull()))
                {
                    m_map.get_tile_at(x, y)->shot();
                }
                // Allow player to shoot again
                if(m_players.end() != m_players.find(pindex))
                {
                    m_players[pindex]->m_shot_allowed = true;
                    m_players[pindex]->m_shot = NULL;
                    if(pindex == m_current_player)
                    {
                        m_shot_fired = false;
                    }
                }
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
        refptr<Player> player = m_players[m_current_player];
        sf::Uint8 w_pressed = KEY_NOT_PRESSED;
        sf::Uint8 a_pressed = KEY_NOT_PRESSED;
        sf::Uint8 s_pressed = KEY_NOT_PRESSED;
        sf::Uint8 d_pressed = KEY_NOT_PRESSED;
        sf::Int32 rel_mouse_movement = 0;

        // This is a fix so that the mouse will not move outside the window and
        // cause the user to click on another program.
        // Note:  Does not work well with fast movement.
        if(m_client_has_focus)
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
            if (m_mouse_grabbed)
            {
                rel_mouse_movement = sf::Mouse::getPosition(*m_window).x - m_width / 2;
                recenter_cursor();
            }

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

        m_player_dir_x = cos(player->direction);
        m_player_dir_y = sin(player->direction);

        // Send an update to the server if something has changed
        if((player->w_pressed != w_pressed) ||
           (player->a_pressed != a_pressed) ||
           (player->s_pressed != s_pressed) ||
           (player->d_pressed != d_pressed) ||
           (player->rel_mouse_movement !=  rel_mouse_movement))
        {
            sf::Uint8 packet_type = PLAYER_UPDATE;
            client_packet.clear();
            client_packet << packet_type;
            client_packet << m_current_player;
            client_packet << w_pressed;
            client_packet << a_pressed;
            client_packet << s_pressed;
            client_packet << d_pressed;
            client_packet << rel_mouse_movement;

            m_net_client->sendData(client_packet);

            player->w_pressed = w_pressed;
            player->a_pressed = a_pressed;
            player->s_pressed = s_pressed;
            player->d_pressed = d_pressed;
            player->rel_mouse_movement =  rel_mouse_movement;
        }
    }
    else if(!registered_player)
    {
        // Needs to be 32 bit so that the packet << overload will work.
        sf::Uint16 players_port = m_net_client->getLocalPort();
        sf::Uint8 packet_type = PLAYER_CONNECT;
        client_packet.clear();
        client_packet << packet_type;
        client_packet << m_current_player;
        client_packet << m_current_player_name;
        // Send the players port.  This will server as a unique
        // identifier and prevent users with the same name from controlling
        // each other.
        client_packet << players_port;
        m_net_client->sendData(client_packet, true);
        registered_player = true;
    }
    else
    {
        // Do nothing.
    }
    m_net_client->Transmit();
}

void Client::create_shot()
{
    if (m_players.size() == 0)
        return;
    sf::Packet client_packet;
    double shot_distance = m_drawing_shot_distance + SHOT_RING_WIDTH / 2.0;
    sf::Uint8 packet_type = PLAYER_SHOT;
    client_packet.clear();
    client_packet << packet_type;
    client_packet << m_current_player;
    client_packet << shot_distance;
    m_net_client->sendData(client_packet, true);
    m_drawing_shot_distance = 0;
    m_players[m_current_player]->m_shot_allowed = false;
}
