#pragma once
#include <SDL3/SDL.h>
#include <string>

class Window
{
public:
    Window() = default;
    ~Window();

    bool create(const std::string& title, int width, int height, SDL_WindowFlags flags = 0);

    SDL_Window* get() const { return window; }

    int getWidth() const { return w; }
    int getHeight() const { return h; }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

private:
    SDL_Window* window = nullptr;
    int w = 0;
    int h = 0;
};
