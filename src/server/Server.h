#include "Network.h"
#include "Player.h"
#include "refptr.h"

class Server{
    public:
        Server(sf::Uint16 port);
        ~Server();
        void run( void );

    protected:
        void update(double elapsed_time);
        refptr<Network> m_net_server;
        refptr<Player> m_player;
        sf::Clock m_clock;
};
