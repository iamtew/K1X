#pragma once

#include <cmath>
#include <cstdint>

namespace k1x
{

// 909-ish analog kick: sine body with pitch drop + noise click, then drive/filters.
// Not a circuit model. Constants below are the tuning knobs if it sits off on real hardware.
struct KickVoice
{
    struct Params
    {
        float tuneHz     = 50.f;
        float pitchDrop  = 1.5f;   // octaves above tune at t=0
        float pitchDecay = 0.04f;  // seconds
        float click      = 0.4f;
        float punch      = 0.25f;  // 0 = ~0.3ms attack, 1 = ~12ms
        float decay      = 0.35f;  // amp decay seconds
        float drive      = 0.2f;
        float hpfHz      = 25.f;
        float toneHz     = 3500.f;
        float lo         = 0.55f;  // 0.5 = flat, 0/1 = ±12 dB
        float hi         = 0.45f;
        float level      = 0.75f;  // applied after the voice (live master)
        float lengthSec  = 0.f;    // 0 = free (follow decay)
    };

    void prepare (double sampleRate)
    {
        sr = sampleRate > 1.0 ? sampleRate : 44100.0;
        resetFilters();
    }

    void trigger (const Params& p)
    {
        params = p;
        pos = 0;
        phase = 0.0;
        noise = 0xAce1u;
        active_ = true;
        // Keep filter state on retrigger — zeroing it pops.

        const float attMs = 0.3f + params.punch * 11.7f;
        attackN = (int) (sr * (double) attMs * 0.001);
        if (attackN < 1)
            attackN = 1;

        // ponytail: free length = ~8 decays then stop; snap length wins if set.
        float life = params.decay * 8.f;
        if (life < 0.05f)
            life = 0.05f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        totalN = (int) (sr * (double) life);
        if (totalN < attackN + 32)
            totalN = attackN + 32;

        // ~25 ms cosine fade: 5 ms is a quarter-cycle at 50 Hz and clicks.
        fadeN = (int) (sr * 0.025);
        if (fadeN < 8)
            fadeN = 8;
        if (fadeN > totalN / 4)
            fadeN = totalN / 4;
    }

    float process()
    {
        if (! active_)
            return 0.f;

        const double t = (double) pos / sr;
        // 909 pitch envelope is roughly exponential down to the tuned fundamental.
        const float startHz = params.tuneHz * std::pow (2.f, params.pitchDrop);
        const float freq = params.tuneHz
            + (startHz - params.tuneHz) * std::exp ((float) (-t / (double) params.pitchDecay));
        phase += (double) freq / sr;
        if (phase >= 1.0)
            phase -= std::floor (phase);
        const float body = std::sin ((float) (phase * 6.283185307179586));

        float amp = 0.f;
        if (pos < attackN)
            amp = (float) pos / (float) attackN;
        else
            amp = std::exp ((float) (-(t - (double) attackN / sr) / (double) params.decay));

        // Click: ~4 ms noise burst, same attack as the body so sample 0 is not a pop.
        const float clickEnv = std::exp ((float) (-t / 0.004));
        noise = noise * 1664525u + 1013904223u;
        const float n = ((float) (int) (noise >> 8) * (1.f / 8388608.f)) - 1.f;
        clickLp += 0.35f * (n - clickLp);
        const float clickSig = (n - clickLp) * clickEnv * params.click * amp;

        float x = body * amp + clickSig;
        // Drive gain sits around 1..5 so tanh actually saturates.
        x = std::tanh (x * (1.f + params.drive * 4.f));

        const float hpC = onePoleCoeff (params.hpfHz);
        const float lpC = onePoleCoeff (params.toneHz);
        hpZ += hpC * (x - hpZ);
        const float hp = x - hpZ;
        lpZ += lpC * (hp - lpZ);

        const float loG = dbToGain ((params.lo - 0.5f) * 24.f);
        const float hiG = dbToGain ((params.hi - 0.5f) * 24.f);
        loZ += lpC * (lpZ - loZ);
        const float loft = loZ * loG + (lpZ - loZ);
        hiZ += hpC * (loft - hiZ);
        float y = (loft - hiZ) * hiG + hiZ;

        if (pos >= totalN - fadeN)
        {
            const int left = totalN - pos;
            const float u = left <= 0 ? 0.f : (float) left / (float) fadeN;
            y *= 0.5f * (1.f - std::cos (3.141592653589793f * u));
        }

        ++pos;
        if (pos >= totalN)
            active_ = false;
        return y;
    }

    bool active() const { return active_; }

    Params params;

private:
    float onePoleCoeff (float hz) const
    {
        const float ny = (float) (sr * 0.45);
        if (hz < 1.f)
            hz = 1.f;
        if (hz > ny)
            hz = ny;
        return 1.f - std::exp (-6.283185307179586f * hz / (float) sr);
    }

    static float dbToGain (float db)
    {
        return std::pow (10.f, db / 20.f);
    }

    void resetFilters()
    {
        hpZ = lpZ = loZ = hiZ = clickLp = 0.f;
    }

    double sr = 44100.0;
    double phase = 0.0;
    int pos = 0;
    int totalN = 0;
    int attackN = 1;
    int fadeN = 8;
    bool active_ = false;
    std::uint32_t noise = 1;
    float hpZ = 0, lpZ = 0, loZ = 0, hiZ = 0, clickLp = 0;
};

} // namespace k1x
