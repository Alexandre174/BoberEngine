#pragma once

#include <string>

struct SDL_Window;

class Level;

class BoberEngine
{
public:
    void setCurrentLevel(Level* level);

    void setWindowTitle(const char* title);

    void startGame();
    void shutDown();
    void stopGame() { isRunning = false; }

private:
    bool isRunning = false;
    Level* currentLevel = nullptr;

    std::string windowTitle = "BoberEngineGame"; // defoult window name
    SDL_Window* sdlWindow = nullptr;
};
