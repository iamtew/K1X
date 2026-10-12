#pragma once

#include "dsp/KickRender.h"
#include "dsp/KickVoice.h"
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

private:
    float raw (const char* id) const;
    int lengthChoice() const;

    k1x::KickVoice voice;
    std::atomic<bool> pendingTrig { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (K1XAudioProcessor)
};
