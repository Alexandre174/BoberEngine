#include "Input.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>

#include <algorithm>
#include <iostream>

namespace
{
    template <typename T>
    T clamp(T v, T lo, T hi)
    {
        return (v < lo) ? lo : (v > hi) ? hi : v;
    }

    bool mapKey(SDL_Scancode sc, Input::Key& out)
    {
        switch (sc)
        {
        case SDL_SCANCODE_A:     out = Input::Key::A;     return true;
        case SDL_SCANCODE_D:     out = Input::Key::D;     return true;
        case SDL_SCANCODE_W:     out = Input::Key::W;     return true;
        case SDL_SCANCODE_S:     out = Input::Key::S;     return true;
        case SDL_SCANCODE_LEFT:  out = Input::Key::Left;  return true;
        case SDL_SCANCODE_RIGHT: out = Input::Key::Right; return true;
        case SDL_SCANCODE_UP:    out = Input::Key::Up;    return true;
        case SDL_SCANCODE_DOWN:  out = Input::Key::Down;  return true;
        case SDL_SCANCODE_SPACE: out = Input::Key::Space; return true;
        case SDL_SCANCODE_ESCAPE: out = Input::Key::Escape; return true;
        default: return false;
        }
    }

    bool mapButton(SDL_GamepadButton btn, Input::PadButton& out)
    {
        switch (btn)
        {
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: out = Input::PadButton::RightShoulder; return true;
        case SDL_GAMEPAD_BUTTON_SOUTH:          out = Input::PadButton::South;         return true;
        default: return false;
        }
    }

    bool mapAxis(SDL_GamepadAxis ax, Input::PadAxis& out)
    {
        switch (ax)
        {
        case SDL_GAMEPAD_AXIS_LEFTX: out = Input::PadAxis::LeftX; return true;
        case SDL_GAMEPAD_AXIS_LEFTY: out = Input::PadAxis::LeftY; return true;
        default: return false;
        }
    }
}

Input& Input::Get()
{
    static Input instance;
    return instance;
}

Input::Input()
{
    keys.fill(false);
    prevKeys.fill(false);
    buttons.fill(false);
    prevButtons.fill(false);
    axes.fill(0.0f);
}

Input::~Input()
{
    closeGamepad();
}

void Input::beginFrame()
{
    prevKeys = keys;
    prevButtons = buttons;
}

void Input::handleEvent(const SDL_Event& e)
{
    switch (e.type)
    {
    case SDL_EVENT_QUIT:
        quit = true;
        break;

    case SDL_EVENT_KEY_DOWN:
    {
        if (e.key.repeat) break;
        Key k;
        if (mapKey(e.key.scancode, k))
            keys[static_cast<size_t>(k)] = true;
        break;
    }

    case SDL_EVENT_KEY_UP:
    {
        Key k;
        if (mapKey(e.key.scancode, k))
            keys[static_cast<size_t>(k)] = false;
        break;
    }

    case SDL_EVENT_GAMEPAD_ADDED:
        if (!gamepad)
            openGamepad(static_cast<int32_t>(e.gdevice.which));
        break;

    case SDL_EVENT_GAMEPAD_REMOVED:
        if (gamepad && static_cast<int32_t>(e.gdevice.which) == gamepadInstanceId)
            closeGamepad();
        break;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    {
        PadButton b;
        if (mapButton(static_cast<SDL_GamepadButton>(e.gbutton.button), b))
            buttons[static_cast<size_t>(b)] = true;
        break;
    }

    case SDL_EVENT_GAMEPAD_BUTTON_UP:
    {
        PadButton b;
        if (mapButton(static_cast<SDL_GamepadButton>(e.gbutton.button), b))
            buttons[static_cast<size_t>(b)] = false;
        break;
    }

    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
    {
        PadAxis a;
        if (!mapAxis(static_cast<SDL_GamepadAxis>(e.gaxis.axis), a))
            break;

        const float maxv = 32767.0f;
        float v = static_cast<float>(e.gaxis.value);
        v = clamp(v / maxv, -1.0f, 1.0f);
        axes[static_cast<size_t>(a)] = v;
        break;
    }

    default:
        break;
    }
}

bool Input::isKeyDown(Key key) const
{
    return keys[static_cast<size_t>(key)];
}

bool Input::wasKeyPressed(Key key) const
{
    const size_t i = static_cast<size_t>(key);
    return keys[i] && !prevKeys[i];
}


bool Input::isButtonDown(PadButton button) const
{
    return buttons[static_cast<size_t>(button)];
}

bool Input::wasButtonPressed(PadButton button) const
{
    const size_t i = static_cast<size_t>(button);
    return buttons[i] && !prevButtons[i];
}

bool Input::wasButtonReleased(PadButton button) const
{
    const size_t i = static_cast<size_t>(button);
    return !buttons[i] && prevButtons[i];
}

float Input::getAxis(PadAxis axis) const
{
    return axes[static_cast<size_t>(axis)];
}

void Input::openGamepad(int32_t instanceId)
{
    SDL_Gamepad* gp = SDL_OpenGamepad(static_cast<SDL_JoystickID>(instanceId));
    if (!gp)
    {
        std::cerr << "SDL_OpenGamepad failed: " << SDL_GetError() << "\n";
        return;
    }

    gamepad = gp;
    gamepadInstanceId = instanceId;
}

void Input::closeGamepad()
{
    if (!gamepad)
        return;

    SDL_CloseGamepad(gamepad);
    gamepad = nullptr;
    gamepadInstanceId = 0;

    buttons.fill(false);
    prevButtons.fill(false);
    axes.fill(0.0f);
}
