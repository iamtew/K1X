#pragma once

#include "KickVoice.h"

namespace k1x
{

// Offline one-shot. Same voice as the audio thread. Used for waveform + WAV export.
inline void renderKick (float* dest, int n, double sampleRate, const KickVoice::Params& p)
{
    KickVoice v;
    v.prepare (sampleRate);
    v.trigger (p);
    for (int i = 0; i < n; ++i)
        dest[i] = v.process() * p.level;
}

inline float lengthSeconds (int lengthChoice, float bpm)
{
    if (bpm < 20.f)
        bpm = 20.f;
    if (lengthChoice == 1) return 60.f / bpm;        // 1/4
    if (lengthChoice == 2) return 30.f / bpm;        // 1/8
    if (lengthChoice == 3) return 15.f / bpm;        // 1/16
    return 0.f; // free — voice uses decay
}

inline float midiNoteToHz (int note)
{
    return 440.f * std::pow (2.f, (float) (note - 69) / 12.f);
}

} // namespace k1x
