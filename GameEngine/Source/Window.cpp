#include "Window.h"
#include <iostream>

Window::~Window()
{
    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

bool Window::create(const std::string& title, int width, int height, SDL_WindowFlags flags) //create window
{
    w = width;
    h = height;

    window = SDL_CreateWindow(title.c_str(), width, height, flags);
    if (!window)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return false;
    }
    return true;
}
