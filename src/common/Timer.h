#ifndef _TIMER_H
#define _TIMER_H

#include <SFML/System/Clock.hpp>


class Timer
{
public:
    // Initializes the time module
    void Init(void);

    // Updates the time module
    void Update();

    // Returns the current time step
    static sf::Uint32 GetTime(void);

    // Returns the time that elapsed this step
    static float GetStepTime(void);

    // Returns the total time elapsed in milliseconds
    static sf::Uint32 GetTotalTime(void);

    // Returns the total time in seconds as a double
    static double GetTimeDouble(void);

    // Returns the difference between the current time and the given time in seconds
    static float GetElapsedTime(sf::Uint32 baseTime);

private:
    // The clock used to take the time measurements
    sf::Clock myClock;

    // The game speed
    float gameSpeed;

    // The total elapsed time since the start of the game
    static double totalElapsedTime;

    // The time that elapsed this step
    static float stepTime;

    // The number of time steps since the start of the game
    static sf::Uint32 curTimeStep;
};
#endif
