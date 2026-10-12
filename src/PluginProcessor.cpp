#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cstring>
#include <vector>

namespace
{
const juce::Identifier kPatchName { "PATCH_NAME" };
constexpr const char* kInitPatch = "Init Patch";
constexpr const char* kPatchExt = ".k1xp";

juce::File libraryRoot()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
        .getChildFile ("Stupid Systems LLC")
        .getChildFile ("K1X");
}

juce::File ensureDir (juce::File dir)
{
    dir.createDirectory();
    return dir;
}

juce::String legalStem (const juce::String& name)
{
    auto stem = juce::File::createLegalFileName (name.trim());
    if (stem.endsWithIgnoreCase (kPatchExt))
        stem = stem.dropLastCharacters ((int) std::strlen (kPatchExt));
    return stem;
}
} // namespace

K1XAudioProcessor::K1XAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    apvts.state.setProperty (kPatchName, kInitPatch, nullptr);
}

void K1XAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voice.prepare (sampleRate);
    analyser.prepare (sampleRate);
    meter.prepare (sampleRate);
    const int ch = juce::jmax (2, getTotalNumOutputChannels());
    postFx.prepare (sampleRate, samplesPerBlock, ch);
    postEq.prepare (sampleRate, samplesPerBlock, ch);
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
    p.style      = juce::roundToInt (raw (ParamID::style));
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
    int midiNote = -1;
    for (const auto meta : midi)
        if (meta.getMessage().isNoteOn())
        {
            trig = true;
            midiNote = meta.getMessage().getNoteNumber();
        }
    midi.clear();

    if (trig)
    {
        auto p = currentParams();
        if (midiNote >= 0)
            p.tuneHz = k1x::midiNoteToHz (midiNote);
        voice.trigger (p);
    }

    auto* l = buffer.getWritePointer (0);
    const int n = buffer.getNumSamples();
    const float g = raw (ParamID::level);
    for (int i = 0; i < n; ++i)
        l[i] = voice.process() * g;

    for (int ch = 1; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, buffer, 0, 0, n);

    postFx.process (buffer, hostBpm(), apvts);
    k1x::EqBandParam bands[k1x::eqBandCount];
    k1x::readEqBands (apvts, bands);
    postEq.process (buffer, k1x::postEqEnabled (apvts), bands);

    analyser.push (buffer);
    meter.push (buffer);
}

void K1XAudioProcessor::renderWithPost (juce::AudioBuffer<float>& dest, double sampleRate)
{
    const int n = dest.getNumSamples();
    const int chs = dest.getNumChannels();
    dest.clear();
    if (n <= 0 || chs <= 0)
        return;

    std::vector<float> dry ((size_t) n);
    renderShot (dry.data(), n, sampleRate);
    for (int i = 0; i < n; ++i)
        dest.setSample (0, i, dry[(size_t) i]);
    for (int ch = 1; ch < chs; ++ch)
        dest.copyFrom (ch, 0, dest, 0, 0, n);

    k1x::PostFx fx;
    k1x::ParametricEq eq;
    const int block = 512;
    fx.prepare (sampleRate, block, chs);
    eq.prepare (sampleRate, block, chs);
    const float bpm = hostBpm();
    for (int pos = 0; pos < n; )
    {
        const int nThis = juce::jmin (block, n - pos);
        juce::AudioBuffer<float> view (dest.getArrayOfWritePointers(), chs, pos, nThis);
        fx.process (view, bpm, apvts);
        k1x::EqBandParam bands[k1x::eqBandCount];
        k1x::readEqBands (apvts, bands);
        eq.process (view, k1x::postEqEnabled (apvts), bands);
        pos += nThis;
    }
}

juce::File K1XAudioProcessor::patchesDir() const
{
    return ensureDir (libraryRoot().getChildFile ("Patches"));
}

juce::StringArray K1XAudioProcessor::patchNames() const
{
    juce::StringArray names;
    for (const auto& entry : juce::RangedDirectoryIterator (patchesDir(), false, "*.k1xp"))
        names.add (entry.getFile().getFileNameWithoutExtension());
    names.sortNatural();
    return names;
}

juce::String K1XAudioProcessor::patchName() const
{
    return apvts.state.getProperty (kPatchName, kInitPatch).toString();
}

void K1XAudioProcessor::initPatch()
{
    for (auto* base : getParameters())
    {
        auto* param = dynamic_cast<juce::RangedAudioParameter*> (base);
        if (param == nullptr)
            continue;
        param->beginChangeGesture();
        param->setValueNotifyingHost (param->getDefaultValue());
        param->endChangeGesture();
    }
    apvts.state.setProperty (kPatchName, kInitPatch, nullptr);
}

bool K1XAudioProcessor::loadPatchByName (const juce::String& name)
{
    const auto stem = legalStem (name);
    if (stem.isEmpty() || stem == kInitPatch)
    {
        initPatch();
        return true;
    }
    const auto src = patchesDir().getChildFile (stem + kPatchExt);
    auto xml = juce::XmlDocument::parse (src);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;
    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    apvts.state.setProperty (kPatchName, stem, nullptr);
    return true;
}

bool K1XAudioProcessor::savePatchAs (const juce::String& name)
{
    const auto stem = legalStem (name);
    if (stem.isEmpty() || stem == kInitPatch)
        return false;
    apvts.state.setProperty (kPatchName, stem, nullptr);
    auto xml = apvts.copyState().createXml();
    if (xml == nullptr)
        return false;
    return patchesDir().getChildFile (stem + kPatchExt).replaceWithText (xml->toString());
}

bool K1XAudioProcessor::savePatch()
{
    const auto name = patchName();
    if (name.isEmpty() || name == kInitPatch)
        return false;
    return savePatchAs (name);
}

bool K1XAudioProcessor::exportPatch (const juce::File& dest)
{
    auto file = dest.hasFileExtension ("k1xp") ? dest : dest.withFileExtension ("k1xp");
    auto xml = apvts.copyState().createXml();
    if (xml == nullptr)
        return false;
    return file.replaceWithText (xml->toString());
}

bool K1XAudioProcessor::importPatch (const juce::File& src)
{
    const auto stem = legalStem (src.getFileNameWithoutExtension());
    if (stem.isEmpty())
        return false;
    const auto dest = patchesDir().getChildFile (stem + kPatchExt);
    if (src.getFullPathName() != dest.getFullPathName())
    {
        dest.deleteFile();
        if (! src.copyFileTo (dest))
            return false;
    }
    return loadPatchByName (stem);
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
