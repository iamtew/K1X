#pragma once

#include "EqBiquad.h"
#include "../parameters/ParameterIDs.h"

#include <JuceHeader.h>

namespace k1x
{

struct EqBandParam
{
    int type = eqTypePeak;
    float hz = 1000.f;
    float gainDb = 0.f;
    float q = 0.7f;
};

struct ParametricEq
{
    void prepare (double sampleRate, int samplesPerBlock, int numChannels)
    {
        sr = sampleRate > 1.0 ? sampleRate : 44100.0;
        const int ch = juce::jmax (1, numChannels);
        const int n = juce::jmax (samplesPerBlock, 64);
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) n, (juce::uint32) ch };
        for (int i = 0; i < eqBandCount; ++i)
        {
            bands[i].prepare (spec);
            if (bands[i].state != nullptr)
            {
                auto& c = bands[i].state->coefficients;
                if (c.size() < 5)
                    c.resize (5);
                auto* raw = bands[i].state->getRawCoefficients();
                if (raw != nullptr)
                {
                    raw[0] = 1.f;
                    raw[1] = 0.f;
                    raw[2] = 0.f;
                    raw[3] = 0.f;
                    raw[4] = 0.f;
                }
            }
        }
        prepared = true;
    }

    void process (juce::AudioBuffer<float>& buffer, bool on, const EqBandParam* params)
    {
        if (! prepared || ! on || params == nullptr || buffer.getNumSamples() <= 0)
            return;

        auto block = juce::dsp::AudioBlock<float> (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);

        for (int i = 0; i < eqBandCount; ++i)
        {
            // ponytail: Off skips without reset; a mid-note re-enable can click
            if (params[i].type <= eqTypeOff)
                continue;

            const auto bq = eqBiquad (params[i].type, (float) sr, params[i].hz,
                                      params[i].q, params[i].gainDb);
            write (i, bq);
            bands[i].process (ctx);
        }
    }

    double sampleRate() const { return sr; }

private:
    void write (int i, const Biquad& b)
    {
        if (bands[i].state == nullptr || bands[i].state->coefficients.size() < 5)
            return;
        auto* c = bands[i].state->getRawCoefficients();
        if (c == nullptr)
            return;
        c[0] = b.b0;
        c[1] = b.b1;
        c[2] = b.b2;
        c[3] = b.a1;
        c[4] = b.a2;
    }

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> bands[eqBandCount];
    double sr = 44100.0;
    bool prepared = false;
};

struct EqIdSet
{
    const char* on = nullptr;
    const char* const* type = nullptr;
    const char* const* freq = nullptr;
    const char* const* gain = nullptr;
    const char* const* q = nullptr;
};

inline EqIdSet postEqIds()
{
    return { ParamID::postEqOn, ParamID::postEqType, ParamID::postEqFreq,
             ParamID::postEqGain, ParamID::postEqQ };
}

inline EqIdSet layerEqIds (int layer)
{
    return { ParamID::lyEqOn[layer], ParamID::lyEqType[layer], ParamID::lyEqFreq[layer],
             ParamID::lyEqGain[layer], ParamID::lyEqQ[layer] };
}

inline void readEqBands (const juce::AudioProcessorValueTreeState& apvts, const EqIdSet& ids, EqBandParam out[eqBandCount])
{
    auto load = [&] (const char* id) -> float
    {
        auto* v = apvts.getRawParameterValue (id);
        return v != nullptr ? v->load() : 0.f;
    };
    for (int i = 0; i < eqBandCount; ++i)
    {
        out[i].type = juce::roundToInt (load (ids.type[i]));
        out[i].hz = load (ids.freq[i]);
        out[i].gainDb = load (ids.gain[i]);
        out[i].q = load (ids.q[i]);
    }
}

inline void readEqBands (const juce::AudioProcessorValueTreeState& apvts, EqBandParam out[eqBandCount])
{
    readEqBands (apvts, postEqIds(), out);
}

inline bool eqEnabled (const juce::AudioProcessorValueTreeState& apvts, const char* onId)
{
    auto* v = apvts.getRawParameterValue (onId);
    return v != nullptr && v->load() >= 0.5f;
}

inline bool postEqEnabled (const juce::AudioProcessorValueTreeState& apvts)
{
    return eqEnabled (apvts, ParamID::postEqOn);
}

} // namespace k1x
