#include "Network.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

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
        tmpclient.disconnect = DISCONNECTED;
        numclients = addClients(&tmpclient, &current_client);

        if(sf::Socket::Done != net_socket.bind( sf::Socket::AnyPort ))
        {
            std::cout << "Error, could not bind to port\n";
        }
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
    Reset();
    net_socket.unbind();
}

bool Network::getData(sf::Packet& p,  sf::Uint8* sending_client)
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
            if(sending_client != NULL)
            {
                *sending_client = curcl;
            }
            rtn = true;
            break;
        }
    }


    return rtn;
}

bool Network::queueTransmitMessage(Network_Messages_T msg_type , sf::Packet p, Client_t * dest)
{
    bool added_message_to_queue = false;
    // Only queue a message if there are clients to receive it
    if(numclients > 0)
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

        added_message_to_queue = true;
    }

    return added_message_to_queue;
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
        // Set that a client is now connected
        clients[i].disconnect = CONNECTED;
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
                                clients[client_id].ping = network_timer.getElapsedTime().asSeconds() - transmit_queue[msg_id]->TimeStarted;

                                // Need to also register that a ping message was received.
                                transmit_queue[msg_id]->Responses[&clients[client_id]] = network_timer.getElapsedTime().asSeconds();
                                // Received a response, so reset send attempts.
                                clients[client_id].num_send_attempts = 0u;
                            }
                            break;
                        }
                        // Handle an acknowledged guaranteed message
                        case NETWORK_GUARANTEED:
                        {
                            // Set that the message was acknowledged by the client
                            if(MAX_NUM_CLIENTS > client_id)
                            {
                                transmit_queue[msg_id]->Responses[&clients[client_id]] = network_timer.getElapsedTime().asSeconds();

                                // Received a response, so reset send attempts.
                                clients[client_id].num_send_attempts = 0u;
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
    static double ping_timer = network_timer.getElapsedTime().asSeconds();
    double current_time = network_timer.getElapsedTime().asSeconds();

    // Every five seconds, send ping messages
    // Note this time must be longer than the combined
    // timeout and retry count, otherwise a client will never
    // be removed.
    if((current_time - ping_timer) > 1.0)
    {
        ping_timer = current_time;
        sf::Packet response;
        response.clear();
        queueTransmitMessage(NETWORK_PING, response);
    }

    // Set any clients waiting to be removed to the
    // do removal state.  This will get changed in the
    // Transmit loop below if there are any pending messages
    for(int client_ndx = 0; client_ndx < MAX_NUM_CLIENTS; client_ndx++)
    {
        if(clients[client_ndx].disconnect == WAIT_DISCONNECT)
        {
            clients[client_ndx].disconnect = DO_DISCONNECT;
        }
    }

    // Send any pending messages
    while(transmit_queue.find(msg_id) != transmit_queue.end())
    {
        double curTime = network_timer.getElapsedTime().asSeconds();
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
                    message->TimeStarted = network_timer.getElapsedTime().asSeconds();
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
                            if((curTime - iter->second) >= NETWORK_TIMEOUT)
                            {
                                // Determine if a response was already received from this client
                                if(message->Responses.find(iter->first) == message->Responses.end())
                                {
                                    // Resend the message to the client
                                    net_socket.send(message->Data, iter->first->addr, iter->first->port);
                                    message->ClientTimeSent[iter->first] = curTime;

                                    // Keep track of the number of attempts
                                    // if the number of attempts is exceeded, fake a receive message
                                    // and set a timeout disconnect state
                                    iter->first->num_send_attempts++;
                                    if(MAX_NUM_SEND_ATTEMPTS < iter->first->num_send_attempts)
                                    {
                                        // Fake a receive message so that it will complete and be removed from the queue
                                        message->Responses[iter->first] = network_timer.getElapsedTime().asSeconds();
                                        iter->first->disconnect = TIMEOUT_DISCONNECT;
                                    }
                                }
                            }

                            // There are still pending messages for this client
                            // Move back to wait state.
                            if(iter->first->disconnect == DO_DISCONNECT)
                            {
                                iter->first->disconnect = WAIT_DISCONNECT;
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

    // Any clients that still have the disconnect action
    // are now safe to remove (i.e. no longer have pending messages)
    for(int client_ndx = 0; client_ndx < MAX_NUM_CLIENTS; client_ndx++)
    {
        if(clients[client_ndx].disconnect == DO_DISCONNECT)
        {
            // Reset all client information.
            clients[client_ndx].addr = sf::IpAddress();
            clients[client_ndx].port = 0;
            clients[client_ndx].disconnect = DISCONNECTED;
            while(!clients[client_ndx].receive.empty())
            {
                clients[client_ndx].receive.pop();
            }
            // Decrement the number of connected clients.
            if(numclients > 0)
            {
                numclients--;
            }
        }
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
        clients[i].disconnect = DISCONNECTED;
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

bool Network::pendingMessages()
{
    return (transmit_queue.size() > 0);
}

sf::Uint16 Network::getLocalPort()
{
    return net_socket.getLocalPort();
}

void Network::disconnectClient(Client_t* player_client)
{
    clients[findClient(player_client)].disconnect = WAIT_DISCONNECT;
}

Client_t* Network::getClient( sf::Uint8 client_ndx )
{
    Client_t* tmp_client = NULL;
    if(client_ndx < MAX_NUM_CLIENTS)
    {
        tmp_client = &clients[client_ndx];
    }
    return tmp_client;
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

