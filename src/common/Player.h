
#ifndef PLAYER_H
#define PLAYER_H

#include <string>

class Player
{
    public:
        std::string name;
        double x;
        double y;
        double direction; /* 0 = East, M_PI_2 = North, M_PI = West, ... */

        Player();
};

#endif
