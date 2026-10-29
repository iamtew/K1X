# K1X – Cursor Rules

Kick drum VST3 + Standalone (909 / 808 / 707 / Tight / Hard). Not a circuit clone. Tune the one-shot with a fixed chain (drive, HP, LP, lo/hi).

## Stack
- JUCE (Projucer workflow only – no CMake)
- C++17, real-time safe: no allocations, no locks, no system calls in processBlock
- APVTS for all parameters
- Just + justfile

## Layout
src/
 PluginProcessor.h/.cpp
 PluginEditor.h/.cpp
 dsp/KickVoice.h KickRender.h PostFx.h EqBiquad.h ParametricEq.h
 parameters/ParameterIDs.h ParameterLayout.h
 ui/VersionCompare.h EqPage.h
docs/
justfile
K1X.jucer

## Version
- Jucer `version=` is what the binary and About dialog show.
- Git tags must match that string (`0.0.5`).
- `just build-dist` names the zips and MSI from the jucer version.

## Do NOT
- Introduce CMake
- Allocate on the audio thread
- Copy commercial plugin code
