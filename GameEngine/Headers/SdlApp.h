#pragma once
#include <SDL3/SDL.h>

class SdlApp
{
public:
    SdlApp() = default;
    ~SdlApp();

    bool init(Uint32 flags);

    SdlApp(const SdlApp&) = delete;
    SdlApp& operator=(const SdlApp&) = delete;

private:
    bool initialized = false;
};
