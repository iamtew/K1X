# K1X manual

Kick one-shot (909 default). MIDI note-on retriggers. Tune sets the body pitch; MIDI note number is ignored.

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

**Volume** (title bar, right) — live master fader and output spectrum. Turns orange for ~800 ms if the output hits 0 dBFS.

**Init Patch** (title bar) — click for Init / Save / Save As / Export / Import, or the saved list. Chevrons step through patches. Files live in `%AppData%\Stupid Systems LLC\K1X\Patches` as `.k1xp`.

Click **K1X** + version in the top left for About and a GitHub update check.

Standalone: **Play** previews. **Export WAV** writes one shot (44.1 kHz, 16-bit, mono).
