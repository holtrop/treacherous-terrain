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

// In seconds
#define NETWORK_TIMEOUT 1

#define MAX_NUM_SEND_ATTEMPTS 3

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

typedef enum{
    DISCONNECTED,
    CONNECTED,
    TIMEOUT_DISCONNECT,
    WAIT_DISCONNECT,
    DO_DISCONNECT
}Disconnect_States_t;

typedef struct{
    sf::IpAddress addr;
    unsigned short port;
    double ping;
    Disconnect_States_t disconnect;
    sf::Uint8 num_send_attempts;
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
        static sf::Clock network_timer;
        static sf::Uint32 getUniqueMessageId();
        static int addClients(Client_t *client, sf::Uint16 *curcl);
        static int findClient(Client_t *client);
        static bool queueTransmitMessage(Network_Messages_T msg_type , sf::Packet p, Client_t * dest = NULL);
        static Client_t clients[MAX_NUM_CLIENTS];

    public:
        static void Create( sf::Uint16 port, sf::IpAddress address );
        static void Destroy();
        static bool getData(sf::Packet& p, sf::Uint8* sending_client = NULL);
        static bool sendData(sf::Packet& p, bool guaranteed = false);
        static int  getNumConnected();
        static void Transmit();
        static void Receive();
        static void Reset();
        static bool pendingMessages();
        static sf::Uint16 getLocalPort();
        static void disconnectClient(Client_t* player_client);
        static Client_t* getClient( sf::Uint8 client_ndx );
};

sf::Packet& operator <<(sf::Packet& Packet, const Network_Messages_T& NMT);

sf::Packet& operator >>(sf::Packet& Packet, Network_Messages_T& NMT);

#endif
