#include "Server.h"
#include "Types.h"
#include <math.h>

Server::Server(sf::Uint16 port)
{
    m_net_server = new Network();
    m_net_server->Create(port, sf::IpAddress::None);
	m_players = new std::map<sf::Uint8, Player>;
    m_players->clear();
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

    m_net_server->Receive();
    // Handle all received data (only really want the latest)
    while(m_net_server->getData(server_packet))
    {
		sf::Uint8 ptype;
		// Get packet type
		server_packet >> ptype;
		switch(ptype)
		{
			case PLAYER_CONNECT:
			{
				Player p;
				std::string pname;
				sf::Uint8 pindex;
				
				server_packet >> pindex;
				server_packet >> pname;
				// When a player connects, we need to associate
				// that player with a new ID. find first unused id
				// player zero means a player does not exist.
				if(pindex == 0)
				{
					for(pindex = 1u; pindex < 255u; pindex++ )
					{
						if(m_players->end() == m_players->find(pindex))
						{
							break;
						}
					}
					p.name = pname;
					(*m_players)[pindex] = p;
			
					// Send a response to the newly added player
					server_packet.clear();
					server_packet << ptype;
					server_packet << pindex;
					server_packet << p.name;
					m_net_server->sendData(server_packet, true);
				}			
				break;
			}
			case PLAYER_UPDATE:
			{
				// Need to determine the correct player id
				// then update the stored contents.
				sf::Uint8 pindex;
				server_packet >> pindex;
				if(m_players->end() != m_players->find(pindex))
				{
					server_packet >> (*m_players)[pindex].w_pressed;
					server_packet >> (*m_players)[pindex].a_pressed;
					server_packet >> (*m_players)[pindex].s_pressed;
					server_packet >> (*m_players)[pindex].d_pressed;
					server_packet >> (*m_players)[pindex].rel_mouse_movement;
				}
				break;
			}
			case PLAYER_DISCONNECT:
			{
				// This completely removes the player from the game
				// Deletes member from the player list
				break;
			}
			case PLAYER_DEATH:
			{
				// This just forces the player to dissapper from the
				// playing field.
				break;
			}
			default:
			{
				// Just eat the packet
				break;
			}
		}
    }
	
	for(std::map<sf::Uint8, Player>::iterator piter = m_players->begin(); piter !=  m_players->end(); piter++)
	{
		sf::Uint8 pindex = piter->first;
		double direction = (*m_players)[pindex].direction;
		direction -= M_PI * 0.5 * (*m_players)[pindex].rel_mouse_movement / 1000;
		if(direction != (*m_players)[pindex].direction)
		{
			(*m_players)[pindex].direction = direction;
			(*m_players)[pindex].updated = true;
		}
		if (KEY_PRESSED == (*m_players)[pindex].a_pressed)
		{
			direction = (*m_players)[pindex].direction + M_PI_2;
			(*m_players)[pindex].x += cos(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].y += sin(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].updated = true;
		}
		if (KEY_PRESSED == (*m_players)[pindex].d_pressed)
		{
			direction = (*m_players)[pindex].direction - M_PI_2;
			(*m_players)[pindex].x += cos(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].y += sin(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].updated = true;
		}
		if (KEY_PRESSED == (*m_players)[pindex].w_pressed)
		{
			direction = (*m_players)[pindex].direction;
			(*m_players)[pindex].x += cos(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].y += sin(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].updated = true;
		}
		if (KEY_PRESSED == (*m_players)[pindex].s_pressed)
		{
			direction = (*m_players)[pindex].direction + M_PI;
			(*m_players)[pindex].x += cos(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].y += sin(direction) * move_speed * elapsed_time;
			(*m_players)[pindex].updated = true;
		}
		
		server_packet.clear();

		// Send the player update if there were changes
		if((*m_players)[pindex].updated)
		{
			sf::Uint8 ptype = PLAYER_UPDATE;
			server_packet << ptype;
			server_packet << pindex;
			server_packet << (*m_players)[pindex].direction;
			server_packet << (*m_players)[pindex].x;
			server_packet << (*m_players)[pindex].y;
			m_net_server->sendData(server_packet);
			(*m_players)[pindex].updated = false;
	   }
	}
    m_net_server->Transmit();
}
