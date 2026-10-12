#pragma once

#include "ParameterIDs.h"

#include <JuceHeader.h>

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    auto pct = [] (NormalisableRange<float> range)
    {
        return AudioParameterFloatAttributes()
            .withStringFromValueFunction ([range] (float v, int)
            {
                return String (roundToInt (range.convertTo0to1 (v) * 100.f)) + "%";
            })
            .withValueFromStringFunction ([range] (const String& t)
            {
                return range.convertFrom0to1 (jlimit (0.f, 1.f, t.getFloatValue() / 100.f));
            });
    };

    auto hzAttrs = [] (NormalisableRange<float> range)
    {
        return AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int)
            {
                return String (roundToInt (v)) + " Hz";
            })
            .withValueFromStringFunction ([range] (const String& t)
            {
                return jlimit (range.start, range.end, t.getFloatValue());
            });
    };

    auto unit = NormalisableRange<float> (0.f, 1.f);
    auto tuneR = NormalisableRange<float> (30.f, 80.f, 0.1f);
    auto dropR = NormalisableRange<float> (0.f, 3.f, 0.01f);
    auto pDecR = NormalisableRange<float> (0.01f, 0.2f, 0.001f, 0.4f);
    auto decR  = NormalisableRange<float> (0.05f, 2.f, 0.001f, 0.4f);
    auto hpR   = NormalisableRange<float> (10.f, 200.f, 0.1f, 0.4f);
    auto lpR   = NormalisableRange<float> (200.f, 8000.f, 1.f, 0.3f);

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::tune, 1 },
                                                        "Tune", tuneR, 50.f, hzAttrs (tuneR)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::pitchDrop, 1 },
                                                        "Drop", dropR, 1.5f, pct (dropR)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::pitchDecay, 1 },
                                                        "P.Decay", pDecR, 0.04f, pct (pDecR)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::click, 1 },
                                                        "Click", unit, 0.4f, pct (unit)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::punch, 1 },
                                                        "Punch", unit, 0.25f, pct (unit)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::decay, 1 },
                                                        "Decay", decR, 0.35f, pct (decR)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::drive, 1 },
                                                        "Drive", unit, 0.2f, pct (unit)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::hpf, 1 },
                                                        "HPF", hpR, 25.f, hzAttrs (hpR)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::tone, 1 },
                                                        "Tone", lpR, 3500.f, hzAttrs (lpR)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::lo, 1 },
                                                        "Lo", unit, 0.55f, pct (unit)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::hi, 1 },
                                                        "Hi", unit, 0.45f, pct (unit)));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::level, 1 },
                                                        "Volume", unit, 0.75f, pct (unit)));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::length, 1 },
                                                         "Length",
                                                         StringArray { "Free", "1/4", "1/8", "1/16" },
                                                         0));
    return layout;
}
