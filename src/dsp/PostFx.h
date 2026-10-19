#pragma once

#include "../parameters/ParameterIDs.h"
#include "KickRender.h"
#include "OutputScope.h"

#include <JuceHeader.h>
#include <cmath>

namespace k1x
{

// Parallel send: Distortion → Delay → Reverb, all fully wet. Mixer is Dry/FX outside.
struct PostFx
{
    void prepare (double sampleRate, int samplesPerBlock, int numChannels)
    {
        sr = sampleRate > 1.0 ? sampleRate : 44100.0;
        const int ch = juce::jmax (1, numChannels);
        const int n = juce::jmax (samplesPerBlock, 64);
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) n, (juce::uint32) ch };
        reverb.prepare (spec);
        delay.prepare (spec);
        delay.setMaximumDelayInSamples ((int) std::ceil (sr * 2.0) + 8);
        delay.reset();
        preDelay.prepare (spec);
        preDelay.setMaximumDelayInSamples ((int) std::ceil (sr * 1.0) + 8);
        preDelay.reset();
        preDelayIdle = true;
        lp.prepare (spec);
        wet.setSize (ch, n);
        scratch.setSize (ch, n);
        prepared = true;
    }

    void process (juce::AudioBuffer<float>& buffer, float bpm,
                  juce::AudioProcessorValueTreeState& apvts)
    {
        if (! prepared || buffer.getNumSamples() <= 0)
            return;

        ensureSize (buffer);
        const int n = buffer.getNumSamples();
        const int chs = buffer.getNumChannels();
        for (int ch = 0; ch < chs; ++ch)
            wet.copyFrom (ch, 0, buffer, ch, 0, n);

        if (on (apvts, ParamID::fxDstOn))
            processDist (n, chs, apvts);
        if (on (apvts, ParamID::fxDlyOn))
            processDelay (n, chs, bpm, apvts);
        if (on (apvts, ParamID::fxRevOn))
            processReverb (n, chs, bpm, apvts);

        const float dryG = p (apvts, ParamID::fxDry);
        const float fxG = p (apvts, ParamID::fxWet);
        float dryP = 0.f, fxP = 0.f;
        for (int ch = 0; ch < chs; ++ch)
        {
            auto* dry = buffer.getWritePointer (ch);
            const auto* w = wet.getReadPointer (ch);
            for (int i = 0; i < n; ++i)
            {
                const float d = dry[i] * dryG;
                const float f = w[i] * fxG;
                dry[i] = d + f;
                dryP = juce::jmax (dryP, std::abs (d));
                fxP = juce::jmax (fxP, std::abs (f));
            }
        }
        dryMeter.setPeak (dryP);
        fxMeter.setPeak (fxP);
    }

    OutputMeter dryMeter, fxMeter;

private:
    float p (juce::AudioProcessorValueTreeState& apvts, const char* id) const
    {
        auto* v = apvts.getRawParameterValue (id);
        return v != nullptr ? v->load() : 0.f;
    }

    bool on (juce::AudioProcessorValueTreeState& apvts, const char* id) const
    {
        return p (apvts, id) >= 0.5f;
    }

    void processDist (int n, int chs, juce::AudioProcessorValueTreeState& apvts)
    {
        const float drive = 1.f + p (apvts, ParamID::fxDstDrive) * 12.f;
        for (int ch = 0; ch < chs; ++ch)
        {
            auto* s = wet.getWritePointer (ch);
            for (int i = 0; i < n; ++i)
                s[i] = std::tanh (s[i] * drive);
        }
    }

    void processDelay (int n, int chs, float bpm, juce::AudioProcessorValueTreeState& apvts)
    {
        const bool sync = p (apvts, ParamID::fxDlySync) >= 0.5f;
        float delaySec = 0.01f + p (apvts, ParamID::fxDlyTime) * 1.99f;
        if (sync)
        {
            const int div = juce::jlimit (0, 3, juce::roundToInt (p (apvts, ParamID::fxDlyDiv)));
            const float beats = div == 0 ? 0.25f : div == 1 ? 0.5f : div == 2 ? 1.f : 2.f;
            delaySec = beatsToSeconds (beats, bpm);
        }
        const float delaySamp = juce::jlimit (1.f, (float) delay.getMaximumDelayInSamples() - 1,
                                              delaySec * (float) sr);
        delay.setDelay (delaySamp);

        const float fb = p (apvts, ParamID::fxDlyFb) * 0.92f;
        const float cut = 200.f + p (apvts, ParamID::fxDlyCut) * 9800.f;
        *lp.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sr, cut, 0.7f);

        scratch.clear();
        for (int i = 0; i < n; ++i)
        {
            for (int ch = 0; ch < chs; ++ch)
            {
                const float in = wet.getSample (ch, i);
                float d = delay.popSample (ch);
                scratch.setSample (ch, i, d);
                delay.pushSample (ch, in + d * fb);
            }
        }
        auto wetBlock = juce::dsp::AudioBlock<float> (scratch).getSubBlock (0, (size_t) n);
        juce::dsp::ProcessContextReplacing<float> wetCtx (wetBlock);
        lp.process (wetCtx);
        for (int ch = 0; ch < chs; ++ch)
            wet.copyFrom (ch, 0, scratch, ch, 0, n);
    }

    void processReverb (int n, int chs, float bpm, juce::AudioProcessorValueTreeState& apvts)
    {
        applyPredelay (n, chs, bpm, apvts);

        juce::dsp::Reverb::Parameters rp;
        rp.roomSize = p (apvts, ParamID::fxRevSize);
        rp.damping = p (apvts, ParamID::fxRevDamp);
        rp.wetLevel = 1.f;
        rp.dryLevel = 0.f;
        rp.width = p (apvts, ParamID::fxRevWidth);
        rp.freezeMode = 0.f;
        reverb.setParameters (rp);
        auto block = juce::dsp::AudioBlock<float> (wet).getSubBlock (0, (size_t) n);
        if (chs < 2)
        {
            // juce Reverb wants stereo; duplicate then take L
            ensureScratchCh (2, n);
            scratch.copyFrom (0, 0, wet, 0, 0, n);
            scratch.copyFrom (1, 0, wet, 0, 0, n);
            auto stereo = juce::dsp::AudioBlock<float> (scratch).getSubBlock (0, (size_t) n);
            juce::dsp::ProcessContextReplacing<float> ctx (stereo);
            reverb.process (ctx);
            wet.copyFrom (0, 0, scratch, 0, 0, n);
            return;
        }
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        reverb.process (ctx);
    }

    void applyPredelay (int n, int chs, float bpm, juce::AudioProcessorValueTreeState& apvts)
    {
        float delaySec = p (apvts, ParamID::fxRevPredelay) * 0.001f;
        if (p (apvts, ParamID::fxRevSync) >= 0.5f)
        {
            const int div = juce::jlimit (0, 3, juce::roundToInt (p (apvts, ParamID::fxRevDiv)));
            const float beats = div == 0 ? 0.0625f : div == 1 ? 0.125f : div == 2 ? 0.25f : 0.5f;
            delaySec = beatsToSeconds (beats, bpm);
        }
        if (delaySec < 0.0005f)
        {
            if (! preDelayIdle)
            {
                preDelay.reset();
                preDelayIdle = true;
            }
            return;
        }
        preDelayIdle = false;
        const float delaySamp = juce::jlimit (1.f, (float) preDelay.getMaximumDelayInSamples() - 1,
                                              delaySec * (float) sr);
        preDelay.setDelay (delaySamp);
        for (int i = 0; i < n; ++i)
        {
            for (int ch = 0; ch < chs; ++ch)
            {
                const float in = wet.getSample (ch, i);
                wet.setSample (ch, i, preDelay.popSample (ch));
                preDelay.pushSample (ch, in);
            }
        }
    }

    void ensureSize (const juce::AudioBuffer<float>& buffer)
    {
        // ponytail: grow only if host exceeds prepare size
        const int n = buffer.getNumSamples();
        const int ch = buffer.getNumChannels();
        if (wet.getNumSamples() < n || wet.getNumChannels() < ch)
            wet.setSize (ch, n, false, false, true);
        if (scratch.getNumSamples() < n || scratch.getNumChannels() < juce::jmax (2, ch))
            scratch.setSize (juce::jmax (2, ch), n, false, false, true);
    }

    void ensureScratchCh (int ch, int n)
    {
        if (scratch.getNumSamples() < n || scratch.getNumChannels() < ch)
            scratch.setSize (ch, n, false, false, true);
    }

    using DupIIR = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                  juce::dsp::IIR::Coefficients<float>>;

    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelay { 192000 };
    DupIIR lp;
    juce::AudioBuffer<float> wet;
    juce::AudioBuffer<float> scratch;
    double sr = 44100.0;
    bool prepared = false;
    bool preDelayIdle = true;
};

} // namespace k1x
