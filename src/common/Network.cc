#include "Network.h"
#include "Timer.h"
#include <cstring>
#include <cstdlib>
#include <iostream>


sf::Uint16 Network::numclients;
sf::UdpSocket  Network::net_socket;
char Network::rxbuff[RECEIVE_BUFFER_SIZE];
std::map<sf::Uint32, Transmit_Message_t*>  Network::transmit_queue;
Client_t Network::clients[MAX_NUM_CLIENTS];
sf::Clock Network::message_timer;

sf::Uint32 Network::getUniqueMessageId( void )
{
    sf::Uint32 next_msg_uid = 0;
    while(transmit_queue.find(next_msg_uid) != transmit_queue.end())
    {
        next_msg_uid++;
    }
    return next_msg_uid;
}

void Network::Create(sf::Uint16 port, sf::IpAddress address )
{
    sf::Uint16 current_client = 0;
    Client_t tmpclient;

    Reset();

    if(sf::IpAddress::None != address)
    {
        tmpclient.addr = address;
        tmpclient.port = port;
        numclients = addClients(&tmpclient, &current_client);

        net_socket.bind( sf::Socket::AnyPort );
        net_socket.setBlocking(false);
    }
    else
    {
        net_socket.bind( port );
        net_socket.setBlocking(false);
    }
}

void Network::Destroy( void )
{
    /* Clean and exit */
    net_socket.unbind();
}

bool Network::getData(sf::Packet& p)
{
    bool rtn = false;
    sf::Uint16 curcl;

    // Recurse through the client array and return if there is any
    // received data for a given client
    for(curcl = 0; curcl < numclients; (curcl)++)
    {
        if(!clients[curcl].receive.empty())
        {
            p = clients[curcl].receive.front();
            clients[curcl].receive.pop();
            rtn = true;
            break;
        }
    }


    return rtn;
}

bool Network::queueTransmitMessage(Network_Messages_T msg_type , sf::Packet p, Client_t * dest)
{
    sf::Packet packet;
    sf::Uint8 type = (sf::Uint8)msg_type;
    sf::Uint32 uid = UNIQUE_ID;
    sf::Uint32 msg_id = getUniqueMessageId();
    Transmit_Message_t* message = new Transmit_Message_t();

    // Add this to the list of packets expecting a response
    packet << uid;
    packet << type;
    packet << msg_id;
    packet.append(p.getData(), p.getDataSize());

    message->msg_type = msg_type;
    message->Data = packet;
    message->TimeStarted = 0.0;
    message->dest = dest;

    transmit_queue[msg_id] = message;

    return true;
}

bool Network::sendData(sf::Packet& p, bool guaranteed)
{
    Network_Messages_T message_type = NETWORK_NORMAL;
    if(guaranteed)
    {
        message_type = NETWORK_GUARANTEED;
    }

    queueTransmitMessage(message_type, p);

    return true;
}

int Network::addClients(Client_t *client, sf::Uint16 *curcl)
{
    int i;
    int nc = -1;
    for(i=0;i<MAX_NUM_CLIENTS;i++)
    {
        if((clients[i].addr == client->addr) && (clients[i].port == client->port))
        {
            nc = i;
        }
        else if((clients[i].addr == sf::IpAddress::None) && (clients[i].port == 0))
        {
            break;
        }
    }
    // Make sure to set the current client location, otherwise
    // bad stuffs can happen.
    if(nc == -1)
    {
        clients[i].addr = client->addr;
        clients[i].port = client->port;
        *curcl = i;
        i++;
    }
    else
    {
        *curcl = nc;
    }
    return i;
}

int Network::findClient(Client_t *client)
{
    int client_ndx;
    for(client_ndx = 0; client_ndx < MAX_NUM_CLIENTS; client_ndx++)
    {
        if((clients[client_ndx].addr == client->addr) && (clients[client_ndx].port == client->port))
        {
            break;
        }
    }
    return client_ndx;
}


void Network::Receive()
{
    // Get any received packets
    sf::IpAddress sender;
    Client_t tmpclient;
    sf::Uint16 curcl;
    sf::Packet receive_packet;
    receive_packet.clear();

    // Receive any packets from the server
    while(net_socket.receive(receive_packet, tmpclient.addr, tmpclient.port) == sf::Socket::Done)
    {
        sf::Uint32 uid;
        receive_packet >> uid;
        if(uid == UNIQUE_ID)
        {
            sf::Uint8 message_type;
            sf::Uint32 msg_id;
            receive_packet >> message_type;
            receive_packet >> msg_id;

            numclients = addClients(&tmpclient, &curcl);

            switch((Network_Messages_T)message_type)
            {
                case NETWORK_CONNECT:
                {
                    break;
                }
                case NETWORK_DISCONNECT:
                {
                    break;
                }
                case NETWORK_ACK:
                {
                    sf::Uint32 client_id = findClient(&tmpclient);
                    receive_packet >> message_type;
                    receive_packet >> msg_id;

                    switch(message_type)
                    {
                        // Handle an acknowledged ping message
                        case NETWORK_PING:
                        {
                            if(MAX_NUM_CLIENTS > client_id)
                            {
                                clients[client_id].ping = Timer::GetTimeDouble() - transmit_queue[msg_id]->TimeStarted;
                            }
                            break;
                        }
                        // Handle an acknowledged guaranteed message
                        case NETWORK_GUARANTEED:
                        {
                            // Set that the message was acknowledged by the client
                            if(MAX_NUM_CLIENTS > client_id)
                            {
                                transmit_queue[msg_id]->Responses[&clients[client_id]] = Timer::GetTimeDouble();
                            }
                            break;
                        }

                        default:
                            break; // What the heck happened?
                    }
                    break;
                }
                case NETWORK_PING:
                {
                    // Send a response indicating that ping was received
                    sf::Packet response;
                    response.clear();
                    response << message_type << msg_id;
                    queueTransmitMessage(NETWORK_ACK, response, &(clients[curcl]));
                    break;
                }

                case NETWORK_NORMAL:
                {
                    // Handle any remaining data in the packet
                    clients[curcl].receive.push(receive_packet);
                    break;
                }

                case NETWORK_GUARANTEED:
                {
                    // Send a response indicating that the message was received
                    sf::Packet response;
                    clients[curcl].receive.push(receive_packet);
                    response.clear();
                    response << message_type << msg_id;
                    queueTransmitMessage(NETWORK_ACK, response, &(clients[curcl]));
                    break;
                }

                // Nothing to do
                default:
                    break;
            }
        }
    }
}

void Network::Transmit()
{
    // Broadcast the mesages to all clients
    sf::Uint32 msg_id = 0;
    static double ping_timer = Timer::GetTimeDouble();

    // Once per second, send ping messages ( just for fun )
    if((Timer::GetTimeDouble() - ping_timer) > 1000.0)
    {
        ping_timer = Timer::GetTimeDouble();
        sf::Packet response;
        response.clear();
        queueTransmitMessage(NETWORK_PING, response);
    }

    // Send any pending messages
    while(transmit_queue.find(msg_id) != transmit_queue.end())
    {
        double curTime = Timer::GetTimeDouble();
        Transmit_Message_t * message = transmit_queue[msg_id];
        switch(message->msg_type)
        {
            case NETWORK_PING:
            case NETWORK_GUARANTEED:
            {
                // If the message has not yet been sent
                // send the message and update the sent times.
                if(0.0 == message->TimeStarted)
                {
                    message->TimeStarted = Timer::GetTimeDouble();
                    for(int i = 0; i < MAX_NUM_CLIENTS; i++)
                    {
                        if((clients[i].addr != sf::IpAddress::None) && (clients[i].port != 0))
                        {
                            message->ClientTimeSent[&clients[i]] = message->TimeStarted;
                            net_socket.send(message->Data, clients[i].addr, clients[i].port);
                        }
                    }
                }
                else
                {
                    // Message has already been sent, check to see if a response has been received
                    // if not, send another request after retry timer expired.
                    if(message->ClientTimeSent.size() > message->Responses.size())
                    {
                        // Look for any clients that haven't responded
                        for ( std::map<Client_t*, double>::iterator iter = message->ClientTimeSent.begin(); iter != message->ClientTimeSent.end(); ++iter )
                        {
                            // Determine if enough time has elapsed to try and re-send
                            if((curTime - iter->second) >= 0.5)
                            {
                                // Determine if a response was already received from this client
                                if(message->Responses.find(iter->first) == message->Responses.end())
                                {
                                    // Resend the message to the client
                                    net_socket.send(message->Data, iter->first->addr, iter->first->port);
                                    message->ClientTimeSent[iter->first] = curTime;
                                }
                            }
                        }
                    }
                    else
                    {
                        // All clients have received the message, so remove it from the list
                        delete message;
                        transmit_queue.erase(transmit_queue.find(msg_id));
                    }
                }
                break;
            }

            // When sending an ACK, we only need to send the ACK to the sender.
            case NETWORK_ACK:
            {
                if(NULL != message->dest)
                {
                    net_socket.send(message->Data, message->dest->addr, message->dest->port);
                }
                // Transmitted the message, so remove it from the list
                delete message;
                transmit_queue.erase(transmit_queue.find(msg_id));
                break;
            }

            default:
            {
                // A normal message, no response needed
                // just send to all clients, then delete the message
                for(int i=0;i<numclients;i++)
                {
                    net_socket.send(message->Data, clients[i].addr, clients[i].port);
                }
                delete message;
                transmit_queue.erase(transmit_queue.find(msg_id));
                break;
            }
        }
        msg_id++;
    }
}

int Network::getNumConnected( void )
{
    return numclients;
}

void Network::Reset()
{
    numclients = 0;
    for(int i = 0; i < MAX_NUM_CLIENTS; i++)
    {
        clients[i].addr = sf::IpAddress();
        clients[i].port = 0;
        while(!clients[i].receive.empty())
        {
            clients[i].receive.pop();
        }
    }

    sf::Uint32 next_msg_uid = 0;
    while(transmit_queue.find(next_msg_uid) != transmit_queue.end())
    {
        delete transmit_queue[next_msg_uid];
        next_msg_uid++;
    }

    transmit_queue.clear();

    message_timer.restart();
}

sf::Packet& operator <<(sf::Packet& Packet, const Network_Messages_T& NMT)
{
    sf::Uint8 net_msg_t = (sf::Uint8)NMT;
    return Packet << net_msg_t;
}

sf::Packet& operator >>(sf::Packet& Packet, Network_Messages_T& NMT)
{
    sf::Uint8 net_msg_t;
    Packet >> net_msg_t;
    NMT = (Network_Messages_T)net_msg_t;
    return Packet;
}
