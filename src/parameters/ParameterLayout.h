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
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::style, 1 },
                                                         "Style",
                                                         StringArray { "909", "808", "707", "Tight", "Hard" },
                                                         0));

    auto addOn = [&] (const char* id, const char* name, bool def = false)
    {
        layout.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    };
    auto addUnit = [&] (const char* id, const char* name, float def)
    {
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, name, unit, def, pct (unit)));
    };

    addUnit (ParamID::fxDry, "Dry", 1.f);
    addUnit (ParamID::fxWet, "FX", 0.f);
    addOn (ParamID::fxDstOn, "Dist On");
    addUnit (ParamID::fxDstDrive, "Dist Drive", 0.4f);
    addOn (ParamID::fxDlyOn, "Delay On");
    addUnit (ParamID::fxDlyTime, "Delay Time", 0.35f);
    addUnit (ParamID::fxDlyFb, "Delay Fb", 0.35f);
    addUnit (ParamID::fxDlyCut, "Delay Cut", 0.7f);
    addOn (ParamID::fxDlySync, "Delay Sync", true);
    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamID::fxDlyDiv, 1 }, "Delay Div",
        StringArray { "1/16", "1/8", "1/4", "1/2" }, 1));
    addOn (ParamID::fxRevOn, "Reverb On");
    addUnit (ParamID::fxRevSize, "Reverb Size", 0.45f);
    addUnit (ParamID::fxRevDamp, "Reverb Damp", 0.4f);
    addUnit (ParamID::fxRevWidth, "Reverb Width", 0.8f);
    auto preR = NormalisableRange<float> (0.f, 200.f, 0.1f);
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamID::fxRevPredelay, 1 }, "Reverb Predelay", preR, 0.f,
        AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int)
            {
                return String (roundToInt (v)) + " ms";
            })
            .withValueFromStringFunction ([preR] (const String& t)
            {
                return jlimit (preR.start, preR.end, t.getFloatValue());
            })));
    addOn (ParamID::fxRevSync, "Reverb Sync");
    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamID::fxRevDiv, 1 }, "Reverb Div",
        StringArray { "1/64", "1/32", "1/16", "1/8" }, 1));

    const float eqHzDef[ParamID::eqBandCount] = { 80.f, 250.f, 1000.f, 4000.f, 10000.f };
    auto eqHz = NormalisableRange<float> (20.f, 20000.f, 0.01f, 0.3f);
    auto eqQ = NormalisableRange<float> (0.1f, 10.f, 0.01f, 0.4f);
    auto eqGain = NormalisableRange<float> (-12.f, 12.f, 0.01f);
    auto eqTypes = StringArray { "Off", "Peak", "Low S", "High S", "HP", "LP" };
    addOn (ParamID::postEqOn, "Post EQ On");
    for (int b = 0; b < ParamID::eqBandCount; ++b)
    {
        const String n = "Post EQ " + String (b + 1);
        layout.add (std::make_unique<AudioParameterChoice> (
            ParameterID { ParamID::postEqType[b], 1 }, n + " Type", eqTypes, 1));
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::postEqFreq[b], 1 }, n + " Freq", eqHz, eqHzDef[b]));
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::postEqGain[b], 1 }, n + " Gain", eqGain, 0.f));
        layout.add (std::make_unique<AudioParameterFloat> (
            ParameterID { ParamID::postEqQ[b], 1 }, n + " Q", eqQ, 0.7f));
    }

    return layout;
}
