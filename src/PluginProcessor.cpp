#include "PluginProcessor.h"
#include "PluginEditor.h"

K1XAudioProcessor::K1XAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
}

void K1XAudioProcessor::prepareToPlay (double sampleRate, int)
{
    voice.prepare (sampleRate);
}

bool K1XAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono())
        return false;
    return layouts.getMainInputChannelSet().isDisabled();
}

float K1XAudioProcessor::raw (const char* id) const
{
    if (auto* p = apvts.getRawParameterValue (id))
        return p->load();
    return 0.f;
}

int K1XAudioProcessor::lengthChoice() const
{
    return juce::roundToInt (raw (ParamID::length));
}

float K1XAudioProcessor::hostBpm() const
{
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                if (*b > 1.0)
                    return (float) *b;
    return 120.f;
}

k1x::KickVoice::Params K1XAudioProcessor::currentParams() const
{
    k1x::KickVoice::Params p;
    p.tuneHz     = raw (ParamID::tune);
    p.pitchDrop  = raw (ParamID::pitchDrop);
    p.pitchDecay = raw (ParamID::pitchDecay);
    p.click      = raw (ParamID::click);
    p.punch      = raw (ParamID::punch);
    p.decay      = raw (ParamID::decay);
    p.drive      = raw (ParamID::drive);
    p.hpfHz      = raw (ParamID::hpf);
    p.toneHz     = raw (ParamID::tone);
    p.lo         = raw (ParamID::lo);
    p.hi         = raw (ParamID::hi);
    p.level      = raw (ParamID::level);
    p.lengthSec  = k1x::lengthSeconds (lengthChoice(), hostBpm());
    return p;
}

void K1XAudioProcessor::renderShot (float* dest, int n, double sampleRate) const
{
    k1x::renderKick (dest, n, sampleRate, currentParams());
}

void K1XAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    bool trig = pendingTrig.exchange (false, std::memory_order_relaxed);
    for (const auto meta : midi)
        if (meta.getMessage().isNoteOn())
            trig = true;
    midi.clear();

    if (trig)
        voice.trigger (currentParams());

    auto* l = buffer.getWritePointer (0);
    const int n = buffer.getNumSamples();
    for (int i = 0; i < n; ++i)
        l[i] = voice.process();

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, buffer, 0, 0, n);
}

void K1XAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void K1XAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* K1XAudioProcessor::createEditor()
{
    return new K1XAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new K1XAudioProcessor();
}
