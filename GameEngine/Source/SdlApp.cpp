#include "SdlApp.h"
#include <iostream>

SdlApp::~SdlApp()
{
    if (initialized)
    {
        SDL_Quit();
    }
}

bool SdlApp::init(Uint32 flags)
{
    if (initialized) return true;

    if (!SDL_Init(flags))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    initialized = true;
    return true;
}
