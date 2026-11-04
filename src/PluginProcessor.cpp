#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "presets/FactoryPresets.h"

static_assert (ParamID::layerOff == k1x::layerOff, "layer type ids");
static_assert (ParamID::layerSweep == k1x::layerSweep, "layer type ids");
static_assert (ParamID::layerCount == k1x::kMixLayers, "layer count");

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

void setParam (juce::AudioProcessorValueTreeState& apvts, const char* id, float value)
{
    if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
    {
        ranged->beginChangeGesture();
        ranged->setValueNotifyingHost (ranged->convertTo0to1 (value));
        ranged->endChangeGesture();
    }
}

void applyFactory (K1XAudioProcessor& proc, const k1x::FactoryPreset& preset)
{
    proc.initPatch();
    auto& apvts = proc.apvts;
    setParam (apvts, ParamID::tune, preset.tune);
    setParam (apvts, ParamID::pitchDrop, preset.drop);
    setParam (apvts, ParamID::pitchDecay, preset.pDec);
    setParam (apvts, ParamID::click, preset.click);
    setParam (apvts, ParamID::punch, preset.punch);
    setParam (apvts, ParamID::decay, preset.decay);
    setParam (apvts, ParamID::drive, preset.drive);
    setParam (apvts, ParamID::hpf, preset.hpf);
    setParam (apvts, ParamID::tone, preset.tone);
    setParam (apvts, ParamID::lo, preset.lo);
    setParam (apvts, ParamID::hi, preset.hi);
    setParam (apvts, ParamID::level, preset.level);
    setParam (apvts, ParamID::style, (float) preset.style);
    setParam (apvts, ParamID::length, (float) preset.length);

    if (const auto* fx = k1x::findFactoryFx (preset.name))
    {
        setParam (apvts, ParamID::fxDry, fx->dry);
        setParam (apvts, ParamID::fxWet, fx->wet);
        if (fx->dst >= 0.f)
        {
            setParam (apvts, ParamID::fxDstOn, 1.f);
            setParam (apvts, ParamID::fxDstDrive, fx->dst);
            setParam (apvts, ParamID::fxDstType, (float) fx->dstType);
        }
        if (fx->dlyDiv >= 0)
        {
            setParam (apvts, ParamID::fxDlyOn, 1.f);
            setParam (apvts, ParamID::fxDlyDiv, (float) fx->dlyDiv);
            setParam (apvts, ParamID::fxDlyFb, fx->dlyFb);
        }
        if (fx->rev >= 0.f)
        {
            setParam (apvts, ParamID::fxRevOn, 1.f);
            setParam (apvts, ParamID::fxRevSize, fx->rev);
            setParam (apvts, ParamID::fxRevDamp, fx->revDamp);
        }
    }

    if (const auto* eq = k1x::findFactoryEq (preset.name))
    {
        setParam (apvts, ParamID::postEqOn, 1.f);
        setParam (apvts, ParamID::postEqType[0], (float) eq->type);
        setParam (apvts, ParamID::postEqFreq[0], eq->hz);
        setParam (apvts, ParamID::postEqGain[0], eq->db);
        setParam (apvts, ParamID::postEqQ[0], eq->q);
        if (eq->type2 > 0)
        {
            setParam (apvts, ParamID::postEqType[1], (float) eq->type2);
            setParam (apvts, ParamID::postEqFreq[1], eq->hz2);
            setParam (apvts, ParamID::postEqGain[1], eq->db2);
            setParam (apvts, ParamID::postEqQ[1], eq->q2);
        }
    }

    apvts.state.setProperty (kPatchName, preset.name, nullptr);
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
    stack.prepare (sampleRate);
    for (auto& m : layerMeters)
        m.prepare (sampleRate);
    // Host blocks stay within the prepared size. 8192 covers a later, larger callback
    // without allocating on the audio thread.
    const int cap = juce::jmax (samplesPerBlock, 8192);
    layerScratch.setSize (k1x::kMixLayers, cap, false, false, true);
    for (int i = 0; i < k1x::kMixLayers; ++i)
    {
        layerEq[i].prepare (sampleRate, cap, 1);
        layerAnalysers[i].prepare (sampleRate);
    }
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
    p.delayMs    = raw (ParamID::lyDelay[0]);
    p.style      = juce::roundToInt (raw (ParamID::style));
    return p;
}

void K1XAudioProcessor::fillSlots (k1x::SlotVoiceParams* out, float kickHz, float lengthSec) const
{
    for (int s = 0; s < ParamID::slotCount; ++s)
    {
        auto& d = out[s];
        d.type = juce::roundToInt (raw (ParamID::lyType[s]));
        const float delay = raw (ParamID::lyDelay[ParamID::layerOfSlot (s)]);
        d.bass.tuneHz = kickHz * std::pow (2.f, raw (ParamID::lyBassTune[s]) / 12.f);
        d.bass.decay = raw (ParamID::lyBassDecay[s]);
        d.bass.drive = raw (ParamID::lyBassDrive[s]);
        d.bass.hpfHz = raw (ParamID::lyBassHp[s]);
        d.bass.delayMs = delay;
        d.bass.lengthSec = lengthSec;
        d.noise.decay = raw (ParamID::lyNoiseDecay[s]);
        d.noise.hpfHz = raw (ParamID::lyNoiseHp[s]);
        d.noise.lpfHz = raw (ParamID::lyNoiseLp[s]);
        d.noise.delayMs = delay;
        d.noise.lengthSec = lengthSec;
        d.click.pitchHz = raw (ParamID::lyClickPitch[s]);
        d.click.decayMs = raw (ParamID::lyClickDecay[s]);
        d.click.tone = raw (ParamID::lyClickTone[s]);
        d.click.delayMs = delay;
        d.click.lengthSec = lengthSec;
        d.sweep.startHz = raw (ParamID::lySweepStart[s]);
        d.sweep.dropOct = raw (ParamID::lySweepDrop[s]);
        d.sweep.decay = raw (ParamID::lySweepDecay[s]);
        d.sweep.delayMs = delay;
        d.sweep.lengthSec = lengthSec;
    }
}

void K1XAudioProcessor::fillAmps (float* out) const
{
    float gain[k1x::kMixLayers];
    bool mute[k1x::kMixLayers], solo[k1x::kMixLayers], enabled[k1x::kMixLayers], flip[k1x::kMixLayers];
    enabled[0] = true;
    for (int s = 0; s < ParamID::slotCount; ++s)
        enabled[ParamID::layerOfSlot (s)] = juce::roundToInt (raw (ParamID::lyType[s])) != ParamID::layerOff;
    for (int i = 0; i < k1x::kMixLayers; ++i)
    {
        gain[i] = raw (ParamID::lyGain[i]);
        mute[i] = raw (ParamID::lyMute[i]) >= 0.5f;
        solo[i] = raw (ParamID::lySolo[i]) >= 0.5f;
        flip[i] = raw (ParamID::lyFlip[i]) >= 0.5f;
    }
    k1x::layerAmps (gain, mute, solo, enabled, flip, out);
}

void K1XAudioProcessor::renderShot (float* dest, int n, double sampleRate) const
{
    auto kickP = currentParams();
    k1x::SlotVoiceParams slots[ParamID::slotCount];
    fillSlots (slots, kickP.tuneHz, kickP.lengthSec);
    float amp[k1x::kMixLayers];
    fillAmps (amp);

    k1x::LayerStack local;
    local.prepare (sampleRate);
    local.trigger (kickP, slots);

    std::vector<float> chan[k1x::kMixLayers];
    for (int L = 0; L < k1x::kMixLayers; ++L)
        chan[L].assign ((size_t) n, 0.f);
    for (int i = 0; i < n; ++i)
    {
        float layer[k1x::kMixLayers];
        local.tickInto (layer);
        for (int L = 0; L < k1x::kMixLayers; ++L)
            chan[L][(size_t) i] = layer[L];
    }

    const int block = 512;
    for (int L = 0; L < k1x::kMixLayers; ++L)
    {
        k1x::ParametricEq eq;
        eq.prepare (sampleRate, block, 1);
        k1x::EqBandParam bands[k1x::eqBandCount];
        const auto ids = k1x::layerEqIds (L);
        k1x::readEqBands (apvts, ids, bands);
        const bool on = k1x::eqEnabled (apvts, ids.on);
        for (int pos = 0; pos < n; )
        {
            const int nThis = juce::jmin (block, n - pos);
            float* ptrs[1] = { chan[L].data() + pos };
            juce::AudioBuffer<float> view (ptrs, 1, nThis);
            eq.process (view, on, bands);
            pos += nThis;
        }
    }

    const float master = kickP.level;
    for (int i = 0; i < n; ++i)
    {
        float acc = 0.f;
        for (int L = 0; L < k1x::kMixLayers; ++L)
            acc += chan[L][(size_t) i] * amp[L];
        dest[i] = acc * master;
    }
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

    auto kickP = currentParams();
    if (midiNote >= 0)
        kickP.tuneHz = k1x::midiNoteToHz (midiNote);
    if (trig)
    {
        k1x::SlotVoiceParams slots[ParamID::slotCount];
        fillSlots (slots, kickP.tuneHz, kickP.lengthSec);
        stack.trigger (kickP, slots);
    }

    float amp[k1x::kMixLayers];
    fillAmps (amp);
    auto* l = buffer.getWritePointer (0);
    const int n = buffer.getNumSamples();
    const int nEq = juce::jmin (n, layerScratch.getNumSamples());
    for (int i = 0; i < nEq; ++i)
    {
        float layer[k1x::kMixLayers];
        stack.tickInto (layer);
        for (int L = 0; L < k1x::kMixLayers; ++L)
            layerScratch.setSample (L, i, layer[L]);
    }
    for (int L = 0; L < k1x::kMixLayers; ++L)
    {
        float* ptrs[1] = { layerScratch.getWritePointer (L) };
        juce::AudioBuffer<float> view (ptrs, 1, nEq);
        k1x::EqBandParam bands[k1x::eqBandCount];
        const auto ids = k1x::layerEqIds (L);
        k1x::readEqBands (apvts, ids, bands);
        layerEq[L].process (view, k1x::eqEnabled (apvts, ids.on), bands);
        layerAnalysers[L].push (view);
    }
    const float g = kickP.level;
    float peak[k1x::kMixLayers] = {};
    for (int i = 0; i < nEq; ++i)
    {
        float acc = 0.f;
        for (int L = 0; L < k1x::kMixLayers; ++L)
        {
            const float y = layerScratch.getSample (L, i) * amp[L];
            acc += y;
            const float a = std::abs (y);
            if (a > peak[L])
                peak[L] = a;
        }
        l[i] = acc * g;
    }
    for (int i = nEq; i < n; ++i)
        l[i] = 0.f;
    for (int L = 0; L < k1x::kMixLayers; ++L)
        layerMeters[L].setPeak (peak[L]);

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
    for (const auto& preset : k1x::factoryPresets)
        names.add (preset.name);
    juce::StringArray extra;
    for (const auto& entry : juce::RangedDirectoryIterator (patchesDir(), false, "*.k1xp"))
    {
        const auto stem = entry.getFile().getFileNameWithoutExtension();
        if (! names.contains (stem))
            extra.add (stem);
    }
    extra.sortNatural();
    names.addArray (extra);
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
    if (src.existsAsFile())
    {
        auto xml = juce::XmlDocument::parse (src);
        if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
            return false;
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        apvts.state.setProperty (kPatchName, stem, nullptr);
        return true;
    }
    if (const auto* preset = k1x::findFactoryPreset (stem.toRawUTF8()))
    {
        applyFactory (*this, *preset);
        return true;
    }
    return false;
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
    {
        xml->setAttribute ("uiScale", uiScale);
        copyXmlToBinary (*xml, destData);
    }
}

void K1XAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            uiScale = juce::jlimit (0.4f, 1.5f, (float) xml->getDoubleAttribute ("uiScale", uiScale));  // sessions only, patches keep your size
            xml->removeAttribute ("uiScale");  // don't leak into patch saves
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

juce::AudioProcessorEditor* K1XAudioProcessor::createEditor()
{
    return new K1XAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new K1XAudioProcessor();
}
