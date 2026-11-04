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

    void arm (float delayMs, float lifeSec, float attackMs = 0.f)
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
        attackN = (int) (sr * (double) attackMs * 0.001);
        if (attackN < 0)
            attackN = 0;
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

    float attack() const
    {
        if (attackN <= 0 || pos >= attackN)
            return 1.f;
        const float u = (float) pos / (float) attackN;
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
    int attackN = 0;
    int delayLeft = 0;
    bool waiting = false;
};

inline void layerSortBand (float& hp, float& lp)
{
    if (hp > lp)
    {
        const float s = hp;
        hp = lp;
        lp = s;
    }
}

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
        float dropOct = 0.f;
        float decay = 0.5f;
        float drive = 0.15f;
        float hpfHz = 20.f;
        float lpfHz = 8000.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
        float attackMs = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        phase = 0.0;
        hpZ = lpZ = 0.f;
        float life = params.decay * 8.f;
        if (life < 0.05f)
            life = 0.05f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life, params.attackMs);
    }

    void silence() { clock.silence(); }
    bool active() const { return clock.active; }

    float process()
    {
        if (! clock.tick())
            return 0.f;
        const double t = (double) clock.pos / clock.sr;
        const float dec = params.decay < 0.02f ? 0.02f : params.decay;
        const float startHz = params.tuneHz * std::pow (2.f, params.dropOct < 0.f ? 0.f : params.dropOct);
        const float freq = params.tuneHz
            + (startHz - params.tuneHz) * std::exp ((float) (-t / (double) dec));
        phase += (double) freq / clock.sr;
        if (phase >= 1.0)
            phase -= std::floor (phase);
        const float body = std::sin ((float) (phase * 6.283185307179586));
        const float amp = std::exp ((float) (-t / (double) dec));
        float x = std::tanh (body * amp * (1.f + params.drive * 4.f));
        float hp = params.hpfHz;
        float lp = params.lpfHz;
        layerSortBand (hp, lp);
        const float hpC = layerOnePole (hp, clock.sr);
        const float lpC = layerOnePole (lp, clock.sr);
        hpZ += hpC * (x - hpZ);
        const float high = x - hpZ;
        lpZ += lpC * (high - lpZ);
        const float y = lpZ * clock.attack() * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    double phase = 0.0;
    float hpZ = 0.f, lpZ = 0.f;
};

struct NoiseVoice
{
    enum Type { white = 0, pink = 1, brown = 2 };

    struct Params
    {
        float decay = 0.12f;
        float hpfHz = 800.f;
        float lpfHz = 6000.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
        float attackMs = 0.f;
        int type = white;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        noise = 0xAce1u;
        hpZ = lpZ = 0.f;
        pinkB0 = pinkB1 = pinkB2 = 0.f;
        brownZ = 0.f;
        float life = params.decay * 6.f;
        if (life < 0.03f)
            life = 0.03f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life, params.attackMs);
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
        float n = ((float) (int) (noise >> 8) * (1.f / 8388608.f)) - 1.f;
        if (params.type == pink)
        {
            // Paul Kellet economy pink.
            pinkB0 = 0.99765f * pinkB0 + n * 0.0990460f;
            pinkB1 = 0.96300f * pinkB1 + n * 0.2965164f;
            pinkB2 = 0.57000f * pinkB2 + n * 1.0526913f;
            n = pinkB0 + pinkB1 + pinkB2 + n * 0.1848f;
        }
        else if (params.type == brown)
        {
            brownZ += 0.02f * (n - brownZ);
            n = brownZ * 3.5f;
        }
        float hp = params.hpfHz;
        float lp = params.lpfHz;
        layerSortBand (hp, lp);
        const float hpC = layerOnePole (hp, clock.sr);
        const float lpC = layerOnePole (lp, clock.sr);
        hpZ += hpC * (n - hpZ);
        const float high = n - hpZ;
        lpZ += lpC * (high - lpZ);
        const float y = lpZ * amp * clock.attack() * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    std::uint32_t noise = 1;
    float hpZ = 0.f, lpZ = 0.f;
    float pinkB0 = 0.f, pinkB1 = 0.f, pinkB2 = 0.f;
    float brownZ = 0.f;
};

struct ClickVoice
{
    struct Params
    {
        float pitchHz = 2500.f;
        float decayMs = 8.f;
        float tone = 0.35f; // 0 noise, 1 sine
        float hpfHz = 80.f;
        float lpfHz = 16000.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
        float attackMs = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        phase = 0.0;
        noise = 0xB16Bu;
        hpZ = lpZ = 0.f;
        float life = params.decayMs * 0.004f; // ~4 decays, decayMs is milliseconds
        if (life < 0.008f)
            life = 0.008f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life, params.attackMs);
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
        float x = (tone * sine + (1.f - tone) * n) * amp;
        float hp = params.hpfHz;
        float lp = params.lpfHz;
        layerSortBand (hp, lp);
        const float hpC = layerOnePole (hp, clock.sr);
        const float lpC = layerOnePole (lp, clock.sr);
        hpZ += hpC * (x - hpZ);
        const float high = x - hpZ;
        lpZ += lpC * (high - lpZ);
        const float y = lpZ * clock.attack() * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    double phase = 0.0;
    std::uint32_t noise = 1;
    float hpZ = 0.f, lpZ = 0.f;
};

struct SweepVoice
{
    struct Params
    {
        float startHz = 400.f;
        float dropOct = 2.f;
        float decay = 0.08f;
        float drive = 0.f;
        float lpfHz = 8000.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
        float attackMs = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        phase = 0.0;
        lpZ = 0.f;
        float life = params.decay * 6.f;
        if (life < 0.03f)
            life = 0.03f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life, params.attackMs);
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
        float x = std::sin ((float) (phase * 6.283185307179586)) * amp;
        if (params.drive > 0.f)
            x = std::tanh (x * (1.f + params.drive * 4.f));
        const float lpC = layerOnePole (params.lpfHz, clock.sr);
        lpZ += lpC * (x - lpZ);
        const float y = lpZ * clock.attack() * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    double phase = 0.0;
    float lpZ = 0.f;
};

struct SampleVoice
{
    struct Params
    {
        const float* data = nullptr;
        int n = 0;
        double fileSr = 44100.0;
        float hpfHz = 80.f;
        float lpfHz = 16000.f;
        float delayMs = 0.f;
        float lengthSec = 0.f;
        float attackMs = 0.f;
    };

    void prepare (double sampleRate) { clock.prepare (sampleRate); }

    void trigger (const Params& p)
    {
        params = p;
        hpZ = lpZ = 0.f;
        float life = 0.05f;
        if (params.data != nullptr && params.n > 1 && params.fileSr > 1.0)
            life = (float) ((double) params.n / params.fileSr);
        if (life < 0.005f)
            life = 0.005f;
        if (params.lengthSec > 0.f)
            life = params.lengthSec;
        clock.arm (params.delayMs, life, params.attackMs);
    }

    void silence() { clock.silence(); }
    bool active() const { return clock.active; }

    float process()
    {
        if (! clock.tick())
            return 0.f;
        float x = 0.f;
        if (params.data != nullptr && params.n > 1 && params.fileSr > 1.0)
        {
            const double idx = (double) clock.pos * (params.fileSr / clock.sr);
            if (idx >= 0.0 && idx < (double) (params.n - 1))
            {
                const int i0 = (int) idx;
                const float frac = (float) (idx - (double) i0);
                x = params.data[i0] + frac * (params.data[i0 + 1] - params.data[i0]);
            }
        }
        float hp = params.hpfHz;
        float lp = params.lpfHz;
        layerSortBand (hp, lp);
        const float hpC = layerOnePole (hp, clock.sr);
        const float lpC = layerOnePole (lp, clock.sr);
        hpZ += hpC * (x - hpZ);
        const float high = x - hpZ;
        lpZ += lpC * (high - lpZ);
        const float y = lpZ * clock.attack() * clock.fade();
        clock.advance();
        return y;
    }

    Params params;

private:
    LayerClock clock;
    float hpZ = 0.f, lpZ = 0.f;
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

inline constexpr int kMixLayers = 4;
inline constexpr int layerOff = 0;
inline constexpr int layerBass = 1;
inline constexpr int layerNoise = 2;
inline constexpr int layerClick = 3;
inline constexpr int layerSweep = 4;
inline constexpr int layerSample = 5;

struct SlotVoiceParams
{
    int type = layerOff;
    BassVoice::Params bass;
    NoiseVoice::Params noise;
    ClickVoice::Params click;
    SweepVoice::Params sweep;
    SampleVoice::Params sample;
};

// gain is linear. flip true inverts. A solo anywhere mutes every layer that is not soloed.
// Disabled or muted layers contribute 0. The voice still has to be ticked by the caller.
inline void layerSendMix (const float* layer, const float* amp, const float* sendAmt,
                          float& dry, float& send, int n = kMixLayers)
{
    dry = 0.f;
    send = 0.f;
    for (int i = 0; i < n; ++i)
    {
        const float y = layer[i] * amp[i];
        dry += y;
        send += y * sendAmt[i];
    }
}

inline void layerSendMixSplit (const float* layer, const float* amp, const float* sendAmt,
                               const bool* isWide, float& mid, float& wide,
                               float& send, int n = kMixLayers)
{
    mid = 0.f;
    wide = 0.f;
    send = 0.f;
    for (int i = 0; i < n; ++i)
    {
        const float y = layer[i] * amp[i];
        send += y * sendAmt[i];
        if (isWide[i])
            wide += y;
        else
            mid += y;
    }
}

inline void layerAmps (const float* gain, const bool* mute, const bool* solo,
                       const bool* enabled, const bool* flip, float* out, int n = kMixLayers)
{
    bool anySolo = false;
    for (int i = 0; i < n; ++i)
        if (enabled[i] && solo[i])
            anySolo = true;
    for (int i = 0; i < n; ++i)
    {
        const bool hear = enabled[i] && ! mute[i] && (! anySolo || solo[i]);
        out[i] = hear ? gain[i] * (flip[i] ? -1.f : 1.f) : 0.f;
    }
}

struct LayerStack
{
    void prepare (double sampleRate)
    {
        kick.prepare (sampleRate);
        for (int i = 0; i < 3; ++i)
        {
            bass[i].prepare (sampleRate);
            noise[i].prepare (sampleRate);
            click[i].prepare (sampleRate);
            sweep[i].prepare (sampleRate);
            sample[i].prepare (sampleRate);
        }
        const double sr = sampleRate > 1.0 ? sampleRate : 44100.0;
        fadeN = (int) (sr * 0.005);
        if (fadeN < 4)
            fadeN = 4;
        for (int L = 0; L < kMixLayers; ++L)
        {
            last[L] = 0.f;
            residual[L] = 0.f;
            residualI[L] = 0;
        }
    }

    void trigger (const KickVoice::Params& kickP, const SlotVoiceParams* slots)
    {
        // Last mixed sample fades out so a live tail does not step to 0.
        for (int L = 0; L < kMixLayers; ++L)
        {
            residual[L] = last[L];
            residualI[L] = fadeN;
        }
        kick.trigger (kickP);
        for (int s = 0; s < 3; ++s)
        {
            bass[s].silence();
            noise[s].silence();
            click[s].silence();
            sweep[s].silence();
            sample[s].silence();
            if (slots == nullptr)
                continue;
            switch (slots[s].type)
            {
                case layerBass:  bass[s].trigger (slots[s].bass); break;
                case layerNoise: noise[s].trigger (slots[s].noise); break;
                case layerClick: click[s].trigger (slots[s].click); break;
                case layerSweep: sweep[s].trigger (slots[s].sweep); break;
                case layerSample: sample[s].trigger (slots[s].sample); break;
                default: break;
            }
        }
    }

    void tickInto (float* layer)
    {
        layer[0] = kick.process();
        for (int s = 0; s < 3; ++s)
            layer[s + 1] = bass[s].process() + noise[s].process()
                         + click[s].process() + sweep[s].process()
                         + sample[s].process();
        for (int L = 0; L < kMixLayers; ++L)
        {
            if (residualI[L] > 0)
            {
                const float u = (float) residualI[L] / (float) fadeN;
                layer[L] += residual[L] * (0.5f * (1.f - std::cos (3.141592653589793f * u)));
                --residualI[L];
            }
            last[L] = layer[L];
        }
    }

    KickVoice kick;
    BassVoice bass[3];
    NoiseVoice noise[3];
    ClickVoice click[3];
    SweepVoice sweep[3];
    SampleVoice sample[3];

private:
    int fadeN = 4;
    float last[kMixLayers] = {};
    float residual[kMixLayers] = {};
    int residualI[kMixLayers] = {};
};

inline void renderSweep (float* dest, int n, double sampleRate, const SweepVoice::Params& p)
{
    SweepVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process();
}

inline void renderSample (float* dest, int n, double sampleRate, const SampleVoice::Params& p)
{
    SampleVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process();
}

} // namespace k1x
