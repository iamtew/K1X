# K1X spec

This file is the product spec.

Kick one-shot. 909-ish default, plus 808 / 707 / Tight / Hard. Stupid Audio. MIT. Not a circuit clone.

- Tune the one-shot with a fixed chain (drive, HP, LP, lo/hi)
- Extra layers on the mix rail, including a user Sample
- Not a circuit-level 909 clone, not a reorderable FX rack

## Product

- VST3 + Standalone
- Standalone: Play (one-shot), Loop + BPM (hosts hide these; they use MIDI)
- WAV export + Norm (peak −0.1 dBTP, default on); drag waveform for the same WAV + patch
- MIDI note-on retriggers and sets body pitch (A4 = 440 Hz). Tune knob is for Standalone Play, waveform, and WAV export.
- About + GitHub tag check (title-bar name + version)
- Window scale 40–150%, saved with the session
- Patches: `%AppData%\Stupid Audio\K1X\Patches`, extension `.k1xp`
- Jucer `version=` is the binary, About dialog, git tag, and `just build-dist` name. Tags match that string (no `v` prefix).

Default sound: 909-ish kick. Style combo is 909 / 808 / 707 / Tight / Hard. Length snap: Free / 1/4 / 1/8 / 1/16 + Hold.

## Layers

Mix mute / solo / gain / phase, Delay / Pre, Send.

- **Kick** — sine (or triangle) body + pitch envelope + click
- **Bass** — extra pitched sine
- **Noise** — White / Pink / Brown burst
- **Click** — extra beater
- **Sweep** — falling tone
- **Sample** — user WAV/AIFF/FLAC, linear resample, no pitch shift; missing file = silence

Duck / Time: dip Kick body and Bass when the click hits. Extra stem buses: Kick + Slot 1–3 (pre-Width, pre-FX). Main Output is always the full mix.

Waveform preview is the layered mix you hear on Play, before post FX. Editing a slot overlays that layer.

## Voice chain

Click + sine → tanh Drive → HP → Tone LP → lo/hi shelves → length gate. Harm, FM, Vel, Width, Punch / A.Hold / Decay on the kick.

Per-layer EQ. Effects page: Dist → Delay → Reverb send + 5-band post EQ.

## Do not

- Circuit-level 909 clone
- Reorderable FX rack
- CMake (Projucer only)
- Allocations, locks, or system calls in `processBlock`
- Commercial plugin code

## Stack

- JUCE, Projucer workflow only
- C++17, APVTS for all parameters
- Just + justfile
- Jucer `version=` is the binary, About dialog, git tag, and `just build-dist` name.

## UI

Steel-blue chassis. Do not invent another palette.

- `#0f151e` background
- `#1a2332` chassis
- `#b7c5d6` metal
- `#d94214` orange
- `#ffffff` white

`assets/logo.png` is the Affinity export. Source of truth. Never overwrite. `just icon` / `just docs-logo` derive the rest.

## Layout

```
src/
  PluginProcessor.h/.cpp
  PluginEditor.h/.cpp
  dsp/KickVoice.h KickRender.h PostFx.h Distortion.h EqBiquad.h ParametricEq.h
  parameters/ParameterIDs.h ParameterLayout.h
  ui/VersionCompare.h EqPage.h
docs/
justfile
K1X.jucer
```

README stays small. LICENSE is MIT. `docs/` is the gh-pages site.
