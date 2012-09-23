#include "Server.h"
#include "Types.h"
#include <math.h>

Server::Server(sf::Uint16 port)
{
    m_net_server = new Network();
    m_net_server->Create(port, sf::IpAddress::None);
    m_player = new Player();
    m_player->x = 0;
    m_player->y = 0;
    m_player->direction = M_PI_2;
}

Server::~Server()
{
    m_net_server->Destroy();
}

void Server::run( void )
{
    double current_time;
    double elapsed_time;
    double last_time = 0.0;
    while(1)
    {
        current_time = m_clock.getElapsedTime().asSeconds();
        elapsed_time = current_time - last_time;
        update( elapsed_time );
        last_time = current_time;

        // temporary for now.  otherwise this thread consumed way too processing
        sf::sleep(sf::seconds(0.005)); // 5 milli-seconds
    }
}


void Server::update( double elapsed_time )
{
    static Player player_prev;
    const double move_speed = 75.0;
    sf::Packet server_packet;
    static sf::Uint8 w_pressed = KEY_NOT_PRESSED;
    static sf::Uint8 a_pressed = KEY_NOT_PRESSED;
    static sf::Uint8 s_pressed = KEY_NOT_PRESSED;
    static sf::Uint8 d_pressed = KEY_NOT_PRESSED;
    static sf::Int32 rel_mouse_movement = 0;

    m_net_server->Receive();
    // Handle all received data (only really want the latest)
    while(m_net_server->getData(server_packet))
    {
        server_packet >> w_pressed;
        server_packet >> a_pressed;
        server_packet >> s_pressed;
        server_packet >> d_pressed;
        server_packet >> rel_mouse_movement;
    }

    if (KEY_PRESSED == a_pressed)
    {
        double direction = m_player->direction + M_PI_2;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    if (KEY_PRESSED == d_pressed)
    {
        double direction = m_player->direction - M_PI_2;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    if (KEY_PRESSED == w_pressed)
    {
        double direction = m_player->direction;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    if (KEY_PRESSED == s_pressed)
    {
        double direction = m_player->direction + M_PI;
        m_player->x += cos(direction) * move_speed * elapsed_time;
        m_player->y += sin(direction) * move_speed * elapsed_time;
    }
    m_player->direction -= M_PI * 0.5 * rel_mouse_movement / 1000;

    server_packet.clear();

    // Send the player update if there were changes
    if((player_prev.direction != m_player->direction) ||
       (player_prev.x != m_player->x) ||
       (player_prev.y != m_player->y))
    {
        server_packet << m_player->direction;
        server_packet << m_player->x;
        server_packet << m_player->y;
        m_net_server->sendData(server_packet);

        player_prev.direction = m_player->direction;
        player_prev.x = m_player->x;
        player_prev.y = m_player->y;
   }

    m_net_server->Transmit();
}
