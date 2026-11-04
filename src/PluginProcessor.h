#pragma once

#include "dsp/KickRender.h"
#include "dsp/KickVoice.h"
#include "dsp/Layers.h"
#include "dsp/OutputScope.h"
#include "dsp/ParametricEq.h"
#include "dsp/PostFx.h"
#include "parameters/ParameterLayout.h"

#include <JuceHeader.h>
#include <atomic>

class K1XAudioProcessor : public juce::AudioProcessor
{
public:
    K1XAudioProcessor();
    ~K1XAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "K1X"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    void requestTrigger() { pendingTrig.store (true, std::memory_order_relaxed); }
    float hostBpm() const;
    k1x::KickVoice::Params currentParams() const;
    void renderShot (float* dest, int n, double sampleRate) const;
    // Post-EQ, pre-fader. chan[layer] is n samples, owned by the caller.
    void renderLayerChans (float* const* chan, int n, double sampleRate) const;
    void renderBars (juce::AudioBuffer<float>& dest, double sampleRate, float bpm, int bars);
    void setLooping (bool on, float bpm);
    void renderWithPost (juce::AudioBuffer<float>& dest, double sampleRate);
    k1x::EqAnalyser& eqAnalyser() { return analyser; }
    k1x::OutputMeter& outputMeter() { return meter; }
    k1x::OutputMeter& dryMeter() { return postFx.dryMeter; }
    k1x::OutputMeter& fxMeter() { return postFx.fxMeter; }
    k1x::OutputMeter& layerMeter (int layer) { return layerMeters[layer]; }
    k1x::EqAnalyser& layerAnalyser (int layer) { return layerAnalysers[layer]; }

    juce::File patchesDir() const;
    juce::StringArray patchNames() const;
    juce::String patchName() const;
    void initPatch();
    bool loadPatchByName (const juce::String& name);
    bool savePatch();
    bool savePatchAs (const juce::String& name);
    bool exportPatch (const juce::File& dest);
    bool importPatch (const juce::File& src);
    bool loadSlotSample (int slot, const juce::File& file);
    void clearSlotSample (int slot);
    juce::String slotSamplePath (int slot) const;
    void reloadSlotSamples();

    // Editor scale (drag the corner). Saved with the session, not with patches.
    float uiScale = 0.85f;
    double wavSr = 44100.0;
    bool wavNorm = true;

private:
    float raw (const char* id) const;
    int lengthChoice() const;
    void fillSlots (k1x::SlotVoiceParams* out, float kickHz, float lengthSec) const;
    void fillAmps (float* out) const;
    void fillSends (float* out) const;
    void applyVelocity (k1x::KickVoice::Params& p, float* amp, int vel) const;

    std::atomic<bool> loopOn { false };
    std::atomic<float> loopBpm { 140.f };
    std::atomic<bool> loopRestart { false };
    int loopWait = 0;

    k1x::LayerStack stack;
    k1x::OutputMeter layerMeters[k1x::kMixLayers];
    k1x::ParametricEq layerEq[k1x::kMixLayers];
    k1x::EqAnalyser layerAnalysers[k1x::kMixLayers];
    juce::AudioBuffer<float> layerScratch;
    juce::AudioBuffer<float> sendScratch;
    // ponytail: two buffers per slot; a third load while the previous-previous still plays can tear.
    struct SlotSample
    {
        juce::AudioBuffer<float> buf[2];
        double fileSr[2] = { 44100.0, 44100.0 };
        std::atomic<int> live { 0 };
    };
    SlotSample slotSamples[ParamID::slotCount];
    k1x::PostFx postFx;
    k1x::ParametricEq postEq;
    k1x::EqAnalyser analyser;
    k1x::OutputMeter meter;
    std::atomic<bool> pendingTrig { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (K1XAudioProcessor)
};
