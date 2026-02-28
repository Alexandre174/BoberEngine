#pragma once

#include <cstdint>
#include <chrono>
#include <random>

namespace GERand
{
    inline uint32_t ticksMs()
    {
        using clock = std::chrono::steady_clock;
        static const auto start = clock::now();
        const auto now = clock::now();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        return static_cast<uint32_t>(ms);
    }

    inline std::mt19937& rng()
    {
        static std::mt19937 gen(ticksMs() ^ 0xA5A5A5A5u);
        return gen;
    }

    inline int rangeInt(int minInclusive, int maxInclusive)
    {
        std::uniform_int_distribution<int> dist(minInclusive, maxInclusive);
        return dist(rng());
    }

    inline float rangeFloat(float minInclusive, float maxInclusive)
    {
        std::uniform_real_distribution<float> dist(minInclusive, maxInclusive);
        return dist(rng());
    }
}
