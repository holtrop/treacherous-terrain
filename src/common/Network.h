#ifndef _NETWORK_HPP
#define _NETWORK_HPP

#include <map>
#include <SFML/Config.hpp>
#include <SFML/Network.hpp>
#include <SFML/System/Clock.hpp>
#include <vector>
#include <queue>

#define MAX_NUM_CLIENTS 8
#define MAX_NUM_TUBES 4

#define UNIQUE_ID   0xDEADBEEF
#define RECEIVE_BUFFER_SIZE 1024

// In milliseconds
#define NETWORK_TIMEOUT 5000



// The bit indicating if the message requires a response
#define MSG_REQUIRES_RESPONSE_BIT ((sf::Uint16)1 << 15)

// The bit indicating if the message is a transmission or a response
#define MSG_TX_BIT ((sf::Uint16)1 << 14)

typedef enum{
    NETWORK_NONE,
    NETWORK_CONNECT,
    NETWORK_DISCONNECT,
    NETWORK_ACK,
    NETWORK_PING,
    NETWORK_NORMAL,
    NETWORK_GUARANTEED
}Network_Messages_T;


typedef struct{
    sf::IpAddress addr;
    unsigned short port;
    double ping;
    std::queue<sf::Packet> receive;
}Client_t;

typedef struct{
    // The packet
    sf::Packet Data;

    // The type of message that is to be sent.
    Network_Messages_T msg_type;

    // Destination client for an ACK message
    // Perhaps later I will enable the ability to only send updates
    // to specific clients.
    Client_t * dest;

    // The time at which the message was origionally sent
    double TimeStarted;

    // The time at which the message was last sent to each client
    std::map<Client_t*, double> ClientTimeSent;

    // The time at which a response was received from each client
    std::map<Client_t*, double> Responses;
} Transmit_Message_t;

class Network{
    private:
        static sf::Uint16 numclients;
        static sf::UdpSocket  net_socket;
        static std::map<sf::Uint32, Transmit_Message_t*> transmit_queue;
        static char rxbuff[RECEIVE_BUFFER_SIZE];
        static sf::Clock message_timer;
        static sf::Uint32 getUniqueMessageId( void );
        static int addClients(Client_t *client, sf::Uint16 *curcl);
        static int findClient(Client_t *client);
        static bool queueTransmitMessage(Network_Messages_T msg_type , sf::Packet p, Client_t * dest = NULL);

    public:
        static Client_t clients[MAX_NUM_CLIENTS];
        static void Create( sf::Uint16 port, sf::IpAddress address );
        static void Destroy( void );
        static bool getData(sf::Packet& p);
        static bool sendData(sf::Packet& p, bool guaranteed = false);
        static int getNumConnected( void );
        static void Transmit();
        static void Receive();
        static void Reset();
};

sf::Packet& operator <<(sf::Packet& Packet, const Network_Messages_T& NMT);

sf::Packet& operator >>(sf::Packet& Packet, Network_Messages_T& NMT);

#endif
