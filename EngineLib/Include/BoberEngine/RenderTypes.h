#pragma once

#include <cstdint>

// Rendering helpers that are safe to use from the game project.
// (SDL/OpenGL details stay inside the engine.)
enum class FlipMode : uint8_t
{
    None = 0,
    Horizontal = 1,
    Vertical = 2,
    Both = 3,
};
