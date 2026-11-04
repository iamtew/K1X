# K1X manual

Kick one-shot (909 default). MIDI note-on retriggers and sets body pitch from the note (A4 = 440 Hz). Tune knob is for Standalone Play, waveform, and WAV export.

## Knobs

- **Tune** — body frequency.
- **Drop** — how many octaves above Tune the pitch starts.
- **P.Hold** — how long the pitch sits at Drop before it falls. 0 is the old one-stage fall.
- **P.Decay** — how fast the pitch falls to Tune.
- **Click** — noise attack.
- **Attack / Punch** — amp attack time (0.3–50 ms, short = clickier). The waveform draws an Ableton-style fade wedge for that length; it is an indication, not a clip start trim.
- **A.Hold** — how long the amp sits at 1 after Punch, before Decay. 0 is the old attack-then-fall.
- **Decay** — amp tail.
- **Drive** — tanh saturation.
- **HPF** — rumble cut.
- **Tone** — low-pass.
- **Lo / Hi** — shelves (±12 dB around centre).
- **Style** — 909, 808, 707, Tight, Hard. Changes click, pitch/amp envelopes, and a bit of harmonic. Does not reset knobs.
- **Length** — Free, or trim to 1/4, 1/8, 1/16 of host BPM (120 if the host has no tempo).

## Layers

The rail next to the waveform adds Bass, Noise, Click, Sweep, or Sample on top of the kick. Each layer has gain/mute/solo/phase on Mix, a **Delay / Pre** (0–80 ms) before the hit, and its own EQ. A strip under the wave is always 0–80 ms: onset, attack length, mute (M) and solo (S). Shared on extra voices:

- **Length** — how long the voice may speak (ms). Auto follows Decay, or the global Length snap when that is not Free.
- **Attack** — fade-in after Pre (0–30 ms).
- **Pre** — same Delay, labeled here so the layer can sit after the beater.

Bass knobs:

- **Tune** — semitones vs the kick body.
- **Drop** — octaves above Tune at t=0; falls with Decay. 0 is a static sine.
- **Decay / Drive / HPF / Tone** — amp tail, tanh, rumble cut, low-pass.

Noise knobs:

- **Type** — White, Pink, or Brown.
- **Decay** — envelope inside the burst.
- **HPF / LPF** — band for the hiss.

Click knobs:

- **Pitch / Decay / Tone** — sine pitch, burst time, sine vs noise mix.
- **HPF / LPF** — band for the burst.

Sweep knobs:

- **Start / Drop / Decay** — start Hz, octaves down, fall time (also amp).
- **Drive / Tone** — tanh (off at 0) and low-pass.

Sample knobs:

- **Load** — one user file per slot (WAV/AIFF/FLAC). No factory library. Path is saved with the patch; a missing file is silence.
- **HPF / LPF / Length / Pre** — same mix rail as the other types. No pitch shift.

**Volume** (title bar, right) — live master fader and a glance spectrum. Turns orange for ~800 ms if the output hits 0 dBFS.

**Main / Effects** (title bar, next to the patch name) — chevrons or click to switch. Main is the kick. Effects is the send plus post EQ on one page.

## Post FX (Effects page)

Parallel send after the layered mix. **Dry** is the unprocessed mix; **FX** is Distortion → Delay → Reverb. Mix **Send** (per layer, default full) is how much of that layer goes into the send. Old patches stay the same. Turn Dist / Delay / Rev on per block.

- **Dist** — Type (Soft tanh, Hard clip, Fold, Diode), Drive, Tone (post LP), Shape (asymmetry). Separate from the kick Drive knob.
- **Delay** — Time, feedback, low-pass Cut. **Sync** uses host BPM and **Div** (1/16 … 1/2).
- **Reverb** — Predelay (ms), Size, Damp, Width. **Sync** snaps Predelay to 1/64 … 1/8 of host BPM.

## Post EQ (same Effects page)

Five-band parametric under the FX row. Spectrum is the live output after FX and EQ. Drag a node for freq/gain, wheel for Q, double-click to reset a band. **EQ** (left of the rail header) bypasses the whole EQ. **Reset** (right half of that header) restores all bands and EQ on/off to defaults.

**Init Patch** (title bar) — a factory preset shows as Single / Category / Name or Multi / Category / Name Stack. Single is the voice. Multi turns slots on (click about 1 ms early, sub about 4 ms late, sweep under the beater — one recipe per category). Click for Init / Save / Save As / Export / Import, a factory category, or your saved list. Chevrons step the factory bank, then any extra patches. Your files live in `%AppData%\Stupid Systems LLC\K1X\Patches` as `.k1xp`. Saving a factory name stores your copy there; delete that file to hear the factory sound again.

Click **K1X** + version in the top left for About and a GitHub update check.

Standalone: **Play** previews. **Export WAV** writes one shot plus FX tail (44.1 kHz, 16-bit, stereo). The waveform is the layered mix you hear on Play, before post FX. Editing a slot overlays that layer on the mix.

Drag the window corner to scale (40%–150%); the scale is saved with the session.
