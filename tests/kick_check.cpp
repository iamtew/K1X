#include "dsp/Distortion.h"
#include "dsp/EqBiquad.h"
#include "dsp/KickRender.h"
#include "dsp/Layers.h"
#include "presets/FactoryPresets.h"
#include "ui/VersionCompare.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

int main()
{
    k1x::KickVoice::Params p;
    std::vector<float> buf (44100);
    k1x::renderKick (buf.data(), (int) buf.size(), 44100.0, p);

    float peak = 0.f, energy = 0.f;
    for (float s : buf)
    {
        peak = std::max (peak, std::abs (s));
        energy += s * s;
    }
    assert (peak > 0.05f);
    assert (peak < 2.f);
    assert (energy > 1.f);

    p.lengthSec = 0.125f; // 1/16 at 120 BPM
    k1x::renderKick (buf.data(), (int) buf.size(), 44100.0, p);
    float tail = 0.f;
    const int after = (int) (0.125f * 44100.f) + 64;
    for (int i = after; i < (int) buf.size(); ++i)
        tail = std::max (tail, std::abs (buf[(size_t) i]));
    assert (tail < 1e-4f);

    const int gateFrom = (int) (0.015f * 44100.f);
    const int gateTo = (int) (0.125f * 44100.f);
    float maxDelta = 0.f;
    for (int i = gateFrom + 1; i < gateTo && i < (int) buf.size(); ++i)
        maxDelta = std::max (maxDelta, std::abs (buf[(size_t) i] - buf[(size_t) i - 1]));
    assert (maxDelta < 0.05f);

    const float q = k1x::lengthSeconds (1, 120.f);
    assert (std::abs (q - 0.5f) < 1e-4f);
    assert (k1x::lengthSeconds (0, 120.f) == 0.f);
    assert (std::abs (k1x::beatsToSeconds (0.25f, 120.f) - 0.125f) < 1e-4f); // 1/16

    assert (std::abs (k1x::midiNoteToHz (69) - 440.f) < 1e-3f);
    assert (std::abs (k1x::midiNoteToHz (57) - 220.f) < 1e-3f); // A3
    assert (std::abs (k1x::midiNoteToHz (81) - 880.f) < 1e-3f); // A5

    p.lengthSec = 0.f;
    std::vector<float> longBuf (88200);
    p.style = 1; // 808
    k1x::renderKick (longBuf.data(), (int) longBuf.size(), 44100.0, p);
    float boom = 0.f;
    const int late = (int) (1.3f * 44100.f);
    for (int i = late; i < (int) longBuf.size(); ++i)
        boom = std::max (boom, std::abs (longBuf[(size_t) i]));
    assert (boom > 0.01f);

    p.style = 3; // Tight
    k1x::renderKick (longBuf.data(), (int) longBuf.size(), 44100.0, p);
    float tightTail = 0.f;
    for (int i = late; i < (int) longBuf.size(); ++i)
        tightTail = std::max (tightTail, std::abs (longBuf[(size_t) i]));
    assert (tightTail < 1e-4f);
    assert (boom > tightTail);

    // Dry=1 FX=0 is a no-op mix; 0 dB peak EQ is unity at its centre.
    {
        float dry = 0.37f, wet = 0.91f;
        const float mixed = dry * 1.f + wet * 0.f;
        assert (std::abs (mixed - dry) < 1e-6f);
        auto b0 = k1x::eqBiquad (k1x::eqTypePeak, 44100.f, 1000.f, 0.7f, 0.f);
        const float mag0 = k1x::eqMagnitude (&b0, 1, 44100.f, 1000.f);
        assert (std::abs (mag0 - 1.f) < 1e-3f);
        auto b6 = k1x::eqBiquad (k1x::eqTypePeak, 44100.f, 1000.f, 0.7f, 6.f);
        const float mag6 = k1x::eqMagnitude (&b6, 1, 44100.f, 1000.f);
        const float db6 = 20.f * std::log10 (mag6);
        assert (std::abs (db6 - 6.f) < 0.25f);
    }

    assert (std::abs (k1x::punchToAttackMs (0.f) - 0.3f) < 1e-4f);
    assert (std::abs (k1x::punchToAttackMs (1.f) - 50.f) < 1e-4f);

    {
        k1x::KickVoice::Params a;
        a.punch = 0.f;
        std::vector<float> fast (44100), slow (44100);
        k1x::renderKick (fast.data(), (int) fast.size(), 44100.0, a);
        a.punch = 1.f;
        k1x::renderKick (slow.data(), (int) slow.size(), 44100.0, a);
        const int n10 = (int) (0.01f * 44100.f);
        float e0 = 0.f, e1 = 0.f;
        for (int i = 0; i < n10; ++i)
        {
            e0 += fast[(size_t) i] * fast[(size_t) i];
            e1 += slow[(size_t) i] * slow[(size_t) i];
        }
        assert (e1 < e0);
    }

    {
        const float drive = 5.f;
        const float x = 0.4f;
        assert (std::abs (k1x::distortSample (k1x::distSoft, x, drive, 0.5f) - std::tanh (x * drive)) < 1e-6f);
        const float hard = k1x::distortSample (k1x::distHard, 2.f, drive, 0.5f);
        assert (hard <= 1.f && hard >= -1.f);
        assert (std::abs (k1x::distortSample (k1x::distFold, x, drive, 0.5f)
                          - k1x::distortSample (k1x::distSoft, x, drive, 0.5f)) > 0.05f);
        float dc = 0.f;
        for (int i = 0; i < 8; ++i)
            dc += k1x::distortSample (k1x::distSoft, 0.f, drive, 0.2f);
        assert (std::abs (dc) < 1e-5f);
    }

    {
        k1x::KickVoice::Params late = p;
        late.delayMs = 20.f;
        late.lengthSec = 0.f;
        late.style = 0;
        k1x::renderKick (buf.data(), (int) buf.size(), 44100.0, late);
        float pre = 0.f;
        const int preN = (int) (0.015f * 44100.f);
        for (int i = 0; i < preN; ++i)
            pre = std::max (pre, std::abs (buf[(size_t) i]));
        assert (pre < 1.0e-6f);

        k1x::BassVoice::Params bass;
        bass.tuneHz = 40.f;
        bass.decay = 0.4f;
        bass.delayMs = 10.f;
        k1x::renderBass (buf.data(), (int) buf.size(), 44100.0, bass);
        pre = 0.f;
        const int bassPre = (int) (0.008f * 44100.f);
        for (int i = 0; i < bassPre; ++i)
            pre = std::max (pre, std::abs (buf[(size_t) i]));
        assert (pre < 1.0e-6f);
        float body = 0.f;
        for (int i = (int) (0.02f * 44100.f); i < (int) (0.08f * 44100.f); ++i)
            body = std::max (body, std::abs (buf[(size_t) i]));
        assert (body > 0.05f);

        k1x::NoiseVoice::Params nz;
        k1x::renderNoise (buf.data(), (int) buf.size(), 44100.0, nz);
        float nrg = 0.f;
        for (int i = 0; i < 4000; ++i)
            nrg += buf[(size_t) i] * buf[(size_t) i];
        assert (nrg > 0.01f);

        k1x::ClickVoice::Params ck;
        ck.decayMs = 5.f;
        ck.lengthSec = 0.f;
        k1x::renderClickLayer (buf.data(), (int) buf.size(), 44100.0, ck);
        float clickTail = 0.f;
        for (int i = (int) (0.05f * 44100.f); i < (int) buf.size(); ++i)
            clickTail = std::max (clickTail, std::abs (buf[(size_t) i]));
        assert (clickTail < 1.0e-4f);

        k1x::SweepVoice::Params sw;
        k1x::renderSweep (buf.data(), (int) buf.size(), 44100.0, sw);
        float swPeak = 0.f;
        for (int i = 0; i < 8000; ++i)
            swPeak = std::max (swPeak, std::abs (buf[(size_t) i]));
        assert (swPeak > 0.05f);
    }

    assert (k1x::factoryPresetCount >= 60);
    assert (k1x::factoryBankOk());

    std::vector<std::string> tags { "0.0.2", "v0.0.1" };
    assert (k1x::newestNewerTag (tags, "0.0.1") == "0.0.2");
    assert (k1x::newestNewerTag (tags, "0.0.2").empty());

    std::printf ("kick_check ok  peak=%.3f energy=%.1f\n", peak, energy);
    return 0;
}
