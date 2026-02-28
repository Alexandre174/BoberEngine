#pragma once

#include <array>
#include <cstdint>

union SDL_Event;
struct SDL_Gamepad;

class Input
{
public:
    enum class Key : uint8_t
    {
        A, D, W, S,
        Left, Right, Up, Down,
        Space,
        Escape,
        Count
    };

    enum class PadButton : uint8_t
    {
        RightShoulder,
        South,
        Count
    };

    enum class PadAxis : uint8_t
    {
        LeftX,
        LeftY,
        Count
    };

    static Input& Get();

    void beginFrame();
    void handleEvent(const SDL_Event& e);

    bool isKeyDown(Key key) const;
    bool wasKeyPressed(Key key) const;

    bool hasGamepad() const { return gamepad != nullptr; }

    bool isButtonDown(PadButton button) const;
    bool wasButtonPressed(PadButton button) const;
    bool wasButtonReleased(PadButton button) const;

    float getAxis(PadAxis axis) const;

    bool quitRequested() const { return quit; }

private:
    Input();
    ~Input();

    void openGamepad(int32_t instanceId);
    void closeGamepad();

    static constexpr size_t KeyCount = static_cast<size_t>(Key::Count);
    std::array<bool, KeyCount> keys{};
    std::array<bool, KeyCount> prevKeys{};

    static constexpr size_t ButtonCount = static_cast<size_t>(PadButton::Count);
    std::array<bool, ButtonCount> buttons{};
    std::array<bool, ButtonCount> prevButtons{};

    static constexpr size_t AxisCount = static_cast<size_t>(PadAxis::Count);
    std::array<float, AxisCount> axes{};

    SDL_Gamepad* gamepad = nullptr;
    int32_t gamepadInstanceId = 0;

    bool quit = false;
};
