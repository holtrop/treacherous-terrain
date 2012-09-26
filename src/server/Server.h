#include "Network.h"
#include "Player.h"
#include "refptr.h"
#include "SFML/Config.hpp"

class Server{
    public:
        Server(sf::Uint16 port);
        ~Server();
        void run( void );

    protected:
        void update(double elapsed_time);
        refptr<Network> m_net_server;
        std::map<sf::Uint8, refptr<Player> > m_players;
        sf::Clock m_clock;
};
