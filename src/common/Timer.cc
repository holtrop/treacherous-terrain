#include "Timer.h"

// The number of time steps per second
const float STEPS_PER_SECOND = 60.0f;

double Timer::totalElapsedTime;
float Timer::stepTime;
sf::Uint32 Timer::curTimeStep;

void Timer::Init(void)
{
    // Reset the clock
    myClock.restart();

    // Set the time keepers to zero
    myTotalElapsedTime = 0;

    // Reset the game speed
    gameSpeed = 1.0f;
}

void Timer::Update(void)
{
    // Record the time step
    stepTime = ((myClock.getElapsedTime().asSeconds() / 1000.0f) * gameSpeed);
    //stepTime = ((((sf::Window*)screen)->GetFrameTime() / 1000.0f) * gameSpeed);
    myClock.restart();

    // Add the time to the total time
    myTotalElapsedTime += stepTime;
    totalElapsedTime = myTotalElapsedTime;

    // Calculate the game step
    curTimeStep = (sf::Uint32)(totalElapsedTime * STEPS_PER_SECOND);
}

sf::Uint32 Timer::GetTime(void)
{
    return curTimeStep;
}

float Timer::GetStepTime(void)
{
    return stepTime;
}

sf::Uint32 Timer::GetTotalTime(void)
{
    return (sf::Uint32)(totalElapsedTime * 1000.0f);
}

double Timer::GetTimeDouble(void)
{
    return totalElapsedTime;
}

float Timer::GetElapsedTime(sf::Uint32 baseTime)
{
    return (totalElapsedTime - ((double)baseTime / STEPS_PER_SECOND));
}
