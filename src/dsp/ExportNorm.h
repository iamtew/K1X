#pragma once

#include <cmath>

namespace k1x
{

// -0.1 dBTP as linear. Meat Bag export default.
inline constexpr float kExportDbtpLin = 0.988553094656f;

inline void clipSamples (float* const* ch, int nCh, int n)
{
    for (int c = 0; c < nCh; ++c)
    {
        float* s = ch[c];
        if (s == nullptr)
            continue;
        for (int i = 0; i < n; ++i)
            s[i] = s[i] < -1.f ? -1.f : (s[i] > 1.f ? 1.f : s[i]);
    }
}

// Linear 4x interpolator as a cheap true-peak estimate.
inline float truePeak (const float* x, int n)
{
    if (x == nullptr || n <= 0)
        return 0.f;
    float p = std::abs (x[0]);
    for (int i = 0; i < n - 1; ++i)
    {
        const float a = x[i];
        const float d = x[i + 1] - a;
        for (int k = 0; k < 4; ++k)
        {
            const float y = std::abs (a + d * (float) k * 0.25f);
            if (y > p)
                p = y;
        }
    }
    const float last = std::abs (x[n - 1]);
    return last > p ? last : p;
}

inline void normalizeToDbtp (float* const* ch, int nCh, int n, int start, int count)
{
    if (nCh <= 0 || count <= 0)
        return;
    if (start < 0)
        start = 0;
    if (start + count > n)
        count = n - start;
    if (count <= 0)
        return;
    float p = 0.f;
    for (int c = 0; c < nCh; ++c)
        if (ch[c] != nullptr)
        {
            const float tp = truePeak (ch[c] + start, count);
            if (tp > p)
                p = tp;
        }
    if (p < 1.0e-8f)
        return;
    const float g = kExportDbtpLin / p;
    for (int c = 0; c < nCh; ++c)
        if (ch[c] != nullptr)
            for (int i = 0; i < count; ++i)
                ch[c][start + i] *= g;
}

} // namespace k1x
