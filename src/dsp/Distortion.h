#pragma once

#include <cmath>

namespace k1x
{

inline constexpr int distSoft = 0;
inline constexpr int distHard = 1;
inline constexpr int distFold = 2;
inline constexpr int distDiode = 3;

inline float distortCore (int type, float x, float drive)
{
    switch (type)
    {
        case distHard:
        {
            const float y = x * drive;
            return y < -1.f ? -1.f : (y > 1.f ? 1.f : y);
        }
        case distFold:
            return std::sin (x * drive);
        case distDiode:
        {
            const float pos = x > 0.f ? x : 0.f;
            const float neg = x < 0.f ? x : 0.f;
            return std::tanh (pos * drive) + std::tanh (neg * drive * 0.25f);
        }
        default:
            return std::tanh (x * drive);
    }
}

// Shape 0.5 = no bias. Offset is shaped then subtracted so DC stays ~0.
inline float distortSample (int type, float x, float drive, float shape)
{
    const float bias = (shape - 0.5f) * 0.9f;
    return distortCore (type, x + bias, drive) - distortCore (type, bias, drive);
}

} // namespace k1x
