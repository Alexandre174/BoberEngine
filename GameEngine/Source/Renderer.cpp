#include "Renderer.h"

#include <iostream>

#include <glad/glad.h>

Renderer::~Renderer()
{
    if (glContext)
    {
        SDL_GL_DestroyContext(glContext);
        glContext = nullptr;
    }
    sdlWindow = nullptr;
}

bool Renderer::create(SDL_Window* window)
{
    sdlWindow = window;
    if (!sdlWindow)
        return false;

    // Create and bind the OpenGL context
    glContext = SDL_GL_CreateContext(sdlWindow);
    if (!glContext)
    {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        return false;
    }

    if (!SDL_GL_MakeCurrent(sdlWindow, glContext))
    {
        std::cerr << "SDL_GL_MakeCurrent failed: " << SDL_GetError() << "\n";
        return false;
    }

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        std::cerr << "gladLoadGLLoader failed\n";
        return false;
    }

    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(sdlWindow, &w, &h);
    glViewport(0, 0, w, h);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Renderer::clear(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::present()
{
    if (sdlWindow)
        SDL_GL_SwapWindow(sdlWindow);
}
