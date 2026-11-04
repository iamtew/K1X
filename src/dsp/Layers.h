#pragma once

#include "KickVoice.h"

#include <cmath>
#include <cstdint>

namespace k1x
{

// Extra one-shots summed with the kick. Delay is silence before the hit so a
// click can sit ahead of a late sub. Length, when set, replaces the natural life
// the same way KickVoice does. Gain, mute, solo, and polarity are applied outside.

struct LayerClock
{
    void prepare (double sampleRate)
    {
        sr = sampleRate > 1.0 ? sampleRate : 44100.0;
    }

    void arm (float delayMs, float lifeSec)
    {
        pos = 0;
        delayLeft = (int) (sr * (double) delayMs * 0.001);
        if (delayLeft < 0)
            delayLeft = 0;
        waiting = delayLeft > 0;
        if (lifeSec < 0.005f)
            lifeSec = 0.005f;
        totalN = (int) (sr * (double) lifeSec);
        if (totalN < 8)
            totalN = 8;
        fadeN = (int) (sr * 0.01);
        if (fadeN < 4)
            fadeN = 4;
        if (fadeN > totalN / 4)
            fadeN = totalN / 4;
        if (fadeN < 1)
            fadeN = 1;
        active = true;
    }

    void silence()
    {
        active = false;
        waiting = false;
    }

    // True when this sample is part of the hit (not the pre-delay, not finished).
    bool tick()
    {
        if (! active)
            return false;
        if (waiting)
        {
            --delayLeft;
            if (delayLeft > 0)
                return false;
            waiting = false;
        }
        return true;
    }

    float fade() const
    {
        if (pos < totalN - fadeN)
            return 1.f;
        const int left = totalN - pos;
        const float u = left <= 0 ? 0.f : (float) left / (float) fadeN;
        return 0.5f * (1.f - std::cos (3.141592653589793f * u));
    }

    void advance()
    {
        ++pos;
        if (pos >= totalN)
            active = false;
    }

    bool active = false;
    double sr = 44100.0;
    int pos = 0;

private:
    int totalN = 0;
    int fadeN = 4;
    int delayLeft = 0;
    bool waiting = false;
};

inline float layerOnePole (float hz, double sr)
{
    const float ny = (float) (sr * 0.45);
    if (hz < 1.f)
        hz = 1.f;
    if (hz > ny)
        hz = ny;
    return 1.f - std::exp (-6.283185307179586f * hz / (float) sr);
}

struct BassVoice
{
    struct Params
    {
        float tuneHz = 25.f;
        float decay = 0.5f;
        float drive = 0.15f;
        float hpfHz = 20.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        phase = 0.0;
        float life = params.decay * 8.f;
        if (life < 0.05f)
            life = 0.05f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life);
    }

    void silence() { clock.silence(); }
    bool active() const { return clock.active; }

    float process()
    {
        if (! clock.tick())
            return 0.f;
        const double t = (double) clock.pos / clock.sr;
        phase += (double) params.tuneHz / clock.sr;
        if (phase >= 1.0)
            phase -= std::floor (phase);
        const float body = std::sin ((float) (phase * 6.283185307179586));
        const float amp = std::exp ((float) (-t / (double) (params.decay < 0.02f ? 0.02f : params.decay)));
        float x = std::tanh (body * amp * (1.f + params.drive * 4.f));
        const float c = layerOnePole (params.hpfHz, clock.sr);
        hpZ += c * (x - hpZ);
        const float y = (x - hpZ) * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    double phase = 0.0;
    float hpZ = 0.f;
};

struct NoiseVoice
{
    struct Params
    {
        float decay = 0.12f;
        float hpfHz = 800.f;
        float lpfHz = 6000.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        noise = 0xAce1u;
        float life = params.decay * 6.f;
        if (life < 0.03f)
            life = 0.03f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life);
    }

    void silence() { clock.silence(); }
    bool active() const { return clock.active; }

    float process()
    {
        if (! clock.tick())
            return 0.f;
        const double t = (double) clock.pos / clock.sr;
        const float dec = params.decay < 0.01f ? 0.01f : params.decay;
        const float amp = std::exp ((float) (-t / (double) dec));
        noise = noise * 1664525u + 1013904223u;
        const float n = ((float) (int) (noise >> 8) * (1.f / 8388608.f)) - 1.f;
        float hp = params.hpfHz;
        float lp = params.lpfHz;
        if (hp > lp)
        {
            const float s = hp;
            hp = lp;
            lp = s;
        }
        const float hpC = layerOnePole (hp, clock.sr);
        const float lpC = layerOnePole (lp, clock.sr);
        hpZ += hpC * (n - hpZ);
        const float high = n - hpZ;
        lpZ += lpC * (high - lpZ);
        const float y = lpZ * amp * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    std::uint32_t noise = 1;
    float hpZ = 0.f, lpZ = 0.f;
};

struct ClickVoice
{
    struct Params
    {
        float pitchHz = 2500.f;
        float decayMs = 8.f;
        float tone = 0.35f; // 0 noise, 1 sine
        float delayMs = 0.f;
        float lengthSec = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        phase = 0.0;
        noise = 0xB16Bu;
        float life = params.decayMs * 0.004f; // ~4 decays, decayMs is milliseconds
        if (life < 0.008f)
            life = 0.008f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life);
    }

    void silence() { clock.silence(); }
    bool active() const { return clock.active; }

    float process()
    {
        if (! clock.tick())
            return 0.f;
        const double t = (double) clock.pos / clock.sr;
        const float dec = params.decayMs < 0.5f ? 0.0005f : params.decayMs * 0.001f;
        const float amp = std::exp ((float) (-t / (double) dec));
        phase += (double) params.pitchHz / clock.sr;
        if (phase >= 1.0)
            phase -= std::floor (phase);
        const float sine = std::sin ((float) (phase * 6.283185307179586));
        noise = noise * 1664525u + 1013904223u;
        const float n = ((float) (int) (noise >> 8) * (1.f / 8388608.f)) - 1.f;
        const float tone = params.tone < 0.f ? 0.f : (params.tone > 1.f ? 1.f : params.tone);
        const float y = (tone * sine + (1.f - tone) * n) * amp * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    double phase = 0.0;
    std::uint32_t noise = 1;
};

struct SweepVoice
{
    struct Params
    {
        float startHz = 400.f;
        float dropOct = 2.f;
        float decay = 0.08f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        phase = 0.0;
        float life = params.decay * 6.f;
        if (life < 0.03f)
            life = 0.03f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life);
    }

    void silence() { clock.silence(); }
    bool active() const { return clock.active; }

    float process()
    {
        if (! clock.tick())
            return 0.f;
        const double t = (double) clock.pos / clock.sr;
        const float dec = params.decay < 0.01f ? 0.01f : params.decay;
        const float start = params.startHz < 20.f ? 20.f : params.startHz;
        const float endHz = start / std::pow (2.f, params.dropOct < 0.f ? 0.f : params.dropOct);
        const float freq = endHz + (start - endHz) * std::exp ((float) (-t / (double) dec));
        phase += (double) freq / clock.sr;
        if (phase >= 1.0)
            phase -= std::floor (phase);
        const float amp = std::exp ((float) (-t / (double) dec));
        const float y = std::sin ((float) (phase * 6.283185307179586)) * amp * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    double phase = 0.0;
};

inline void renderBass (float* dest, int n, double sampleRate, const BassVoice::Params& p)
{
    BassVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process();
}

inline void renderNoise (float* dest, int n, double sampleRate, const NoiseVoice::Params& p)
{
    NoiseVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process();
}

inline void renderClickLayer (float* dest, int n, double sampleRate, const ClickVoice::Params& p)
{
    ClickVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process();
}

inline void renderSweep (float* dest, int n, double sampleRate, const SweepVoice::Params& p)
{
    SweepVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process();
}

} // namespace k1x
