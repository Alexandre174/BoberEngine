#pragma once

#include <box2d/box2d.h>

namespace GEPhysics
{
    // Same scaling the project has been using.
    constexpr float PixelsPerMeter = 50.0f;

    inline b2Vec2 pixelsToMeters(float px, float py)
    {
        return b2Vec2{ px / PixelsPerMeter, py / PixelsPerMeter };
    }

    inline b2Vec2 pixelsToMetersVec(float px, float py)
    {
        return b2Vec2{ px / PixelsPerMeter, py / PixelsPerMeter };
    }

    inline void metersToPixels(const b2Vec2& m, float& outPx, float& outPy)
    {
        outPx = m.x * PixelsPerMeter;
        outPy = m.y * PixelsPerMeter;
    }
}
