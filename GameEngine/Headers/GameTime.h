#pragma once
#include <SDL3/SDL.h>

class GameTime
{
public:
    GameTime();

    // Call once per frame to update delta time.
    void tick();

    float getDeltaTimeSeconds() const { return deltaTimeSeconds; }

private:
    Uint64 lastCounter = 0;
    Uint64 frequency = 0;
    float deltaTimeSeconds = 0.0f;
};
