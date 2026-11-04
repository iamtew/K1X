#pragma once

#include <cmath>
#include <vector>

namespace k1x
{

// Width 0: L = R = mid + wide (old mono copy). Width > 0: Haas on wide, then
// high-pass the side at 120 Hz so the sub stays mono.
struct StereoWidth
{
    void prepare (double sampleRate)
    {
        sr = sampleRate > 1.0 ? sampleRate : 44100.0;
        const int n = (int) (sr * 0.008) + 8;
        delay.assign ((size_t) n, 0.f);
        w = 0;
        hpZ = 0.f;
        hpC = 1.f - std::exp (-6.283185307179586f * 120.f / (float) sr);
    }

    void process (float mid, float wide, float width, float& L, float& R)
    {
        const float sum = mid + wide;
        if (width <= 0.f || delay.empty())
        {
            L = R = sum;
            return;
        }
        const int n = (int) delay.size();
        int dN = (int) (width * (float) (n - 8));
        if (dN < 1)
            dN = 1;
        int ri = w - dN;
        while (ri < 0)
            ri += n;
        const float delayed = delay[(size_t) ri];
        delay[(size_t) w] = wide;
        ++w;
        if (w >= n)
            w = 0;
        L = mid + wide;
        R = mid + delayed;
        const float m = 0.5f * (L + R);
        float s = 0.5f * (L - R);
        hpZ += hpC * (s - hpZ);
        s -= hpZ;
        L = m + s;
        R = m - s;
    }

    std::vector<float> delay;
    int w = 0;
    float hpZ = 0.f;
    float hpC = 0.f;
    double sr = 44100.0;
};

} // namespace k1x
