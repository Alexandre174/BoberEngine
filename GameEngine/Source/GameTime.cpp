#include "GameTime.h"

GameTime::GameTime()
{
    frequency = SDL_GetPerformanceFrequency();
    lastCounter = SDL_GetPerformanceCounter();
}

void GameTime::tick()
{
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 elapsed = now - lastCounter;
    lastCounter = now;

    if (frequency > 0)
    {
        deltaTimeSeconds = static_cast<float>(static_cast<double>(elapsed) / static_cast<double>(frequency));
    }
    else
    {
        deltaTimeSeconds = 0.0f;
    }
}
