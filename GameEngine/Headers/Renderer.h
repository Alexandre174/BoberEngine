#pragma once
#include <SDL3/SDL.h>

#include <cstdint>

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    bool create(SDL_Window* window);

    void clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    void present();

    SDL_Renderer* get() const { return nullptr; }

    SDL_Window* getWindow() const { return sdlWindow; }

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

private:
    SDL_Window* sdlWindow = nullptr;
    SDL_GLContext glContext = nullptr;
};
