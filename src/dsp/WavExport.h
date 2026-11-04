#pragma once

#include "ExportNorm.h"

#include <JuceHeader.h>

namespace k1x
{

inline bool writeWav (const juce::File& file, juce::AudioBuffer<float>& buf,
                      double sampleRate, int start, int count, bool normalize)
{
    if (count <= 0 || buf.getNumChannels() < 1)
        return false;
    if (start < 0)
        start = 0;
    if (start + count > buf.getNumSamples())
        count = buf.getNumSamples() - start;
    if (count <= 0)
        return false;

    if (normalize)
        normalizeToDbtp (buf.getArrayOfWritePointers(), buf.getNumChannels(),
                         buf.getNumSamples(), start, count);

    juce::WavAudioFormat fmt;
    std::unique_ptr<juce::OutputStream> out (file.createOutputStream());
    if (out == nullptr)
        return false;
    const auto opts = juce::AudioFormatWriterOptions{}
                          .withSampleRate (sampleRate)
                          .withNumChannels (buf.getNumChannels())
                          .withBitsPerSample (24);
    auto w = fmt.createWriterFor (out, opts);
    if (w == nullptr)
        return false;
    return w->writeFromAudioSampleBuffer (buf, start, count);
}

} // namespace k1x
