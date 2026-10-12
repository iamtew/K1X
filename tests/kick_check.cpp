#include "dsp/KickRender.h"
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

    std::vector<std::string> tags { "0.0.2", "v0.0.1" };
    assert (k1x::newestNewerTag (tags, "0.0.1") == "0.0.2");
    assert (k1x::newestNewerTag (tags, "0.0.2").empty());

    std::printf ("kick_check ok  peak=%.3f energy=%.1f\n", peak, energy);
    return 0;
}
