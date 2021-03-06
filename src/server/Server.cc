#include "Server.h"
#include "Types.h"
#include <math.h>

Server::Server(sf::Uint16 port)
{
    m_net_server = new Network();
    m_net_server->Create(port, sf::IpAddress::None);
    m_players.clear();
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
    const double move_speed = 50.0;
    sf::Packet server_packet;
    sf::Uint8 tmp_player_client;

    m_net_server->Receive();
    // Handle all received data (only really want the latest)
    while(m_net_server->getData(server_packet, &tmp_player_client))
    {
        sf::Uint8 ptype;
        // Get packet type
        server_packet >> ptype;
        switch(ptype)
        {
            case PLAYER_CONNECT:
            {
                sf::Uint16 players_port = sf::Socket::AnyPort;
                refptr<Player> p = new Player();
                std::string pname;
                sf::Uint8 pindex;

                server_packet >> pindex;
                server_packet >> pname;
                server_packet >> players_port;
                // When a player connects, we need to associate
                // that player with a new ID. find first unused id
                // player zero means a player does not exist.
                if(pindex == 0)
                {
                    for(pindex = 1u; pindex < 255u; pindex++ )
                    {
                        if(m_players.end() == m_players.find(pindex))
                        {
                            break;
                        }
                    }
                    p->name = pname;
                    p->m_client = m_net_server->getClient(tmp_player_client);
                    m_players[pindex] = p;

                    // Alert all connected clients of all the connected players.
                    for(std::map<sf::Uint8, refptr<Player> >::iterator piter = m_players.begin(); piter !=  m_players.end(); piter++)
                    {
                        sf::Uint16 port = ((piter->first == pindex) ? players_port : 0u);
                        server_packet.clear();
                        server_packet << ptype;
                        server_packet << piter->first;
                        server_packet << piter->second->name;
                        server_packet << port;
                        // Send correct starting locations so that they match
                        // the other players screens.
                        server_packet << piter->second->direction;
                        server_packet << piter->second->x;
                        server_packet << piter->second->y;
                        m_net_server->sendData(server_packet, true);
                    }
                }
                break;
            }
            case PLAYER_UPDATE:
            {
                // Need to determine the correct player id
                // then update the stored contents.
                sf::Uint8 pindex;
                server_packet >> pindex;
                if((m_players.end() != m_players.find(pindex)) &&
                   (m_net_server->getClient(tmp_player_client) == m_players[pindex]->m_client))
                {
                    server_packet >> m_players[pindex]->w_pressed;
                    server_packet >> m_players[pindex]->a_pressed;
                    server_packet >> m_players[pindex]->s_pressed;
                    server_packet >> m_players[pindex]->d_pressed;
                    server_packet >> m_players[pindex]->rel_mouse_movement;
                }
                break;
            }
            case PLAYER_DISCONNECT:
            {
                sf::Uint8 pindex;
                std::size_t num_erased = 0;
                // This completely removes the player from the game
                // Deletes member from the player list
                server_packet >> pindex;
                if((m_players.end() != m_players.find(pindex)) &&
                   (m_net_server->getClient(tmp_player_client) == m_players[pindex]->m_client))
                {
                    // Tell networking code to remove the client.
                    m_net_server->disconnectClient(m_players[pindex]->m_client);
                    num_erased = m_players.erase(pindex);
                    if(1 == num_erased)
                    {
                        // Player exited, alert all connected clients.
                        server_packet.clear();
                        server_packet << ptype;
                        server_packet << pindex;
                        m_net_server->sendData(server_packet, true);
                    }
                }
                break;
            }
            case PLAYER_DEATH:
            {
                // This just forces the player to dissapper from the
                // playing field.
                break;
            }
            
            case PLAYER_SHOT:
            {
                sf::Uint8 pindex;
                double shot_distance;
                server_packet >> pindex;
                server_packet >> shot_distance;
                // start the shot process if a player is allowed to shoot and exits
                if((m_players.end() != m_players.find(pindex)) &&
                   (m_players[pindex]->m_shot_allowed))
                {               
                    // Perhaps sometime in the future, the shots will
                    // be different colors depending on the player
                    // or different power ups and what not.
                    refptr<Shot> shot = new Shot(sf::Vector2f(m_players[pindex]->x, m_players[pindex]->y),
                                                 m_players[pindex]->direction,
                                                 shot_distance);
                    m_players[pindex]->m_shot = shot;
                    m_players[pindex]->m_shot_allowed = false;  
                    
                    // Send a packet to all players that a shot has been fired
                    server_packet.clear();
                    server_packet << ptype;
                    server_packet << pindex;
                    server_packet << m_players[pindex]->x;
                    server_packet << m_players[pindex]->y;
                    server_packet << m_players[pindex]->direction;
                    server_packet << shot_distance;                    
                    m_net_server->sendData(server_packet, true);
                }
                break;
            }
            
            default:
            {
                // Just eat the packet
                break;
            }
        }
    }

    for(std::map<sf::Uint8, refptr<Player> >::iterator piter = m_players.begin(); piter !=  m_players.end();)
    {
        sf::Uint8 pindex = piter->first;
        // Increment to the next element.  This is done in loop so that the
        // erasing of players will not cause issues
        piter++;

        if(m_players[pindex]->m_client->disconnect == CONNECTED)
        {
            /* decrease player hover when not over a tile */
            if((m_players[pindex]->hover > 0) &&
              ((m_map.get_tile_at(m_players[pindex]->x, m_players[pindex]->y).isNull()) ||
               (m_map.get_tile_at(m_players[pindex]->x, m_players[pindex]->y)->get_damage_state() == HexTile::DESTROYED)))
            {
                m_players[pindex]->hover -= elapsed_time / 10;
                if (m_players[pindex]->hover < 0)
                {
                    sf::Uint8 ptype = PLAYER_DEATH;
                    m_players[pindex]->hover = 0;                    
                    // Player is now dead.  
                    m_players[pindex]->m_is_dead = true;    
                    server_packet.clear();
                    server_packet << ptype;
                    server_packet << pindex;
                    m_net_server->sendData(server_packet, true);
                }
                m_players[pindex]->updated = true;
            }

            if(!(m_players[pindex]->m_shot.isNull()))
            {
                // Calculate the distance the projectile travelled so far
                // if the position is below tiles, take the current position
                // and calculate the tile location.
                sf::Vector3f shot_pos = m_players[pindex]->m_shot->get_position();
                if(0.0 > shot_pos.z)
                {
                    // Get tile at shot location.
                    refptr<HexTile> p_tile = m_map.get_tile_at(shot_pos.x, shot_pos.y);
                    // Send a message to all clients letting them know a tile was damaged
                    // always send message since it will reenable the player shot ability.
                    sf::Uint8 ptype = TILE_DAMAGED;                        
                    server_packet.clear();
                    server_packet << ptype;
                    server_packet << shot_pos.x;
                    server_packet << shot_pos.y;
                    server_packet << pindex;  // Needed to alert the client that the player can now shoot again.
                    m_net_server->sendData(server_packet, true);
                    
                    // Reset the shot logic
                    m_players[pindex]->m_shot_allowed = true;
                    
                    // If tile exists, damage the tile.
                    if((!p_tile.isNull()) &&
                       (p_tile->get_damage_state() < HexTile::DESTROYED))
                    {                        
                        p_tile->shot();
                    }
                    
                    // Destroy the shot
                    m_players[pindex]->m_shot = NULL;
                }
            }
            
            // If player is not dead, allow them to move.            
            if(!m_players[pindex]->m_is_dead)
            {
                if (KEY_PRESSED == m_players[pindex]->a_pressed)
                {
                    double direction = m_players[pindex]->direction + M_PI_2;
                    m_players[pindex]->x += cos(direction) * move_speed * elapsed_time;
                    m_players[pindex]->y += sin(direction) * move_speed * elapsed_time;
                    m_players[pindex]->updated = true;
                }
                if (KEY_PRESSED == m_players[pindex]->d_pressed)
                {
                    double direction = m_players[pindex]->direction - M_PI_2;
                    m_players[pindex]->x += cos(direction) * move_speed * elapsed_time;
                    m_players[pindex]->y += sin(direction) * move_speed * elapsed_time;
                    m_players[pindex]->updated = true;
                }
                if (KEY_PRESSED == m_players[pindex]->w_pressed)
                {
                    double direction = m_players[pindex]->direction;
                    m_players[pindex]->x += cos(direction) * move_speed * elapsed_time;
                    m_players[pindex]->y += sin(direction) * move_speed * elapsed_time;
                    m_players[pindex]->updated = true;
                }
                if (KEY_PRESSED == m_players[pindex]->s_pressed)
                {
                    double direction = m_players[pindex]->direction + M_PI;
                    m_players[pindex]->x += cos(direction) * move_speed * elapsed_time;
                    m_players[pindex]->y += sin(direction) * move_speed * elapsed_time;
                    m_players[pindex]->updated = true;
                }
                if(0 != m_players[pindex]->rel_mouse_movement)
                {
                    m_players[pindex]->direction -= M_PI * 0.5 * m_players[pindex]->rel_mouse_movement / 1000;
                    m_players[pindex]->updated = true;
                }
            }
            
            server_packet.clear();

            // Send the player update if there were changes
            if(m_players[pindex]->updated)
            {
                sf::Uint8 ptype = PLAYER_UPDATE;
                server_packet << ptype;
                server_packet << pindex;
                server_packet << m_players[pindex]->direction;
                server_packet << m_players[pindex]->x;
                server_packet << m_players[pindex]->y;
                server_packet << m_players[pindex]->hover;
                m_net_server->sendData(server_packet);
                m_players[pindex]->updated = false;
           }
        }
        else
        {
            if(m_players[pindex]->m_client->disconnect == TIMEOUT_DISCONNECT)
            {
                // Tell networking code to remove the client.
                m_net_server->disconnectClient(m_players[pindex]->m_client);

                if(m_players.erase(pindex))
                {
                    sf::Uint8 ptype = PLAYER_DISCONNECT;
                    // Player exited, alert all connected clients.
                    server_packet.clear();
                    server_packet << ptype;
                    server_packet << pindex;
                    m_net_server->sendData(server_packet, true);
                }
            }
        }
    }
    m_net_server->Transmit();
}
