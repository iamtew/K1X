# K1X manual

Kick one-shot (909 default). MIDI note-on retriggers and sets body pitch from the note (A4 = 440 Hz). Tune knob is for Standalone Play, waveform, and WAV export.

## Knobs

- **Tune** — body frequency.
- **Drop** — how many octaves above Tune the pitch starts.
- **P.Decay** — how fast the pitch falls to Tune.
- **Click** — noise attack.
- **Punch** — amp attack time (short = clickier).
- **Decay** — amp tail.
- **Drive** — tanh saturation.
- **HPF** — rumble cut.
- **Tone** — low-pass.
- **Lo / Hi** — shelves (±12 dB around centre).
- **Style** — 909, 808, 707, Tight, Hard. Changes click, pitch/amp envelopes, and a bit of harmonic. Does not reset knobs.
- **Length** — Free, or trim to 1/4, 1/8, 1/16 of host BPM (120 if the host has no tempo).

**Volume** (title bar, right) — live master fader and a glance spectrum. Turns orange for ~800 ms if the output hits 0 dBFS.

**Main / Effects** (title bar, next to the patch name) — chevrons or click to switch. Main is the kick. Effects is the send plus post EQ on one page.

## Post FX (Effects page)

Parallel send after the kick. **Dry** is the unprocessed kick; **FX** is Distortion → Delay → Reverb. Turn Dist / Delay / Rev on per block.

- **Drive** — extra tanh on the send (separate from the kick Drive knob).
- **Delay** — Time, feedback, low-pass Cut. **Sync** uses host BPM and **Div** (1/16 … 1/2).
- **Reverb** — Predelay (ms), Size, Damp, Width. **Sync** snaps Predelay to 1/64 … 1/8 of host BPM.

## Post EQ (same Effects page)

Five-band parametric under the FX row. Spectrum is the live output after FX and EQ. Drag a node for freq/gain, wheel for Q, double-click to reset a band. **EQ** (left of the rail header) bypasses the whole EQ. **Reset** (right half of that header) restores all bands and EQ on/off to defaults.

**Init Patch** (title bar) — click for Init / Save / Save As / Export / Import, or the saved list. Chevrons step through patches. Files live in `%AppData%\Stupid Systems LLC\K1X\Patches` as `.k1xp`.

Click **K1X** + version in the top left for About and a GitHub update check.

Standalone: **Play** previews. **Export WAV** writes one shot plus FX tail (44.1 kHz, 16-bit, stereo). Waveform preview stays the dry kick.

Drag the window corner to scale (40%–150%); the scale is saved with the session.
