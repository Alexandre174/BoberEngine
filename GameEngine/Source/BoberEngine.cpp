#include "BoberEngine.h"
#include "Level.h"
#include "SdlApp.h"
#include "Window.h"
#include "Renderer.h"
#include "GameTime.h"
#include "Input.h"

#include <iostream>

void BoberEngine::setWindowTitle(const char* title)
{
    if (title && title[0] != '\0')
        windowTitle = title;


    if (sdlWindow)
        SDL_SetWindowTitle(sdlWindow, windowTitle.c_str());
}

void BoberEngine::startGame()
{
    std::cout << "BoberEngine has started...\n";

    if (!currentLevel)
    {
        std::cout << "No current level set!\n";
        return;
    }

    SdlApp sdl;
    if (!sdl.init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        std::cout << "SDL failed to initialize.\n";
        return;
    }

    Window window;

    // OpenGL context settings
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (!window.create(windowTitle, 1280, 720, SDL_WINDOW_OPENGL))
    {
        std::cout << "Window creation failed.\n";
        return;
    }

    // Allow runtime title changes via setWindowTitle().
    sdlWindow = window.get();

    Renderer renderer;
    if (!renderer.create(window.get()))
    {
        std::cout << "Renderer creation failed.\n";
        return;
    }

    // give the window size to the level
    currentLevel->setViewportPixels(window.getWidth(), window.getHeight());
    
    // SDL_Renderer not used anymore, here just becouse
    currentLevel->setRenderer(reinterpret_cast<SDL_Renderer*>(1));

    isRunning = true;
    currentLevel->beginPlay();

    GameTime time;
    Input& input = Input::Get();

    SDL_Event e;
    while (isRunning)
    {
        input.beginFrame();

        // handle sdl events
        while (SDL_PollEvent(&e))
        {

            input.handleEvent(e);

            // let actors and level use events
            currentLevel->handleEvent(e);
        }

        // quit
        if (input.quitRequested())
        {
            isRunning = false;
        }

        if (input.wasKeyPressed(Input::Key::Escape))
        {
            isRunning = false;
        }

        time.tick();
        float deltaTime = time.getDeltaTimeSeconds();

        currentLevel->update(deltaTime);  // update level

        renderer.clear(0, 0, 0, 255); 
        currentLevel->render();           //render level
        renderer.present();
    }

    sdlWindow = nullptr;
}

void BoberEngine::shutDown()
{
    std::cout << "Engine has shut down.\n";
}

void BoberEngine::setCurrentLevel(Level* level)
{
    currentLevel = level;
}
