# Extra layers

The rail next to the waveform adds Bass, Noise, Click, Sweep, or Sample on top of the kick. Think of it as a small drum kit hiding in the one-shot.

Each layer has gain, mute, solo, and phase on Mix, a **Delay / Pre** (0–80 ms) before the hit, and its own EQ (**EQ** in the toolbar). A strip under the wave is always 0–80 ms: onset, attack length, mute (M) and solo (S).

Shared on extra voices:

- **Length** — how long the voice may speak (ms). Auto follows Decay, or the global Length snap when that is not Free.
- **Attack** — fade-in after Pre (0–30 ms).
- **Pre** — same Delay, labeled here so the layer can sit after the beater.

## Bass

- **Tune** — semitones vs the kick body.
- **Drop** — octaves above Tune at t=0; falls with Decay. 0 is a static sine.
- **Decay / Drive / HPF / Tone** — amp tail, tanh, rumble cut, low-pass.

## Noise

- **Type** — White, Pink, or Brown.
- **Decay** — envelope inside the burst.
- **HPF / LPF** — band for the hiss.

## Click

- **Pitch / Decay / Tone** — sine pitch, burst time, sine vs noise mix.
- **HPF / LPF** — band for the burst.

## Sweep

- **Start / Drop / Decay** — start Hz, octaves down, fall time (also amp).
- **Drive / Tone** — tanh (off at 0) and low-pass.

## Sample

- **Load** — one user file per slot (WAV / AIFF / FLAC). No factory library. The path is saved with the patch; a missing file is silence, not a crash, which is the polite option.
- **HPF / LPF / Length / Pre** — same mix rail as the other types. No pitch shift.

The waveform is the layered mix you hear on Play, before post FX. Editing a slot overlays that layer on the mix.
