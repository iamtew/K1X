# Kick knobs

These live on Main. Double-click a knob when you have gone too far. The patch will forgive you.

- **Tune** — body frequency (note name and Hz). MIDI note-on still sets pitch from the note without moving the knob.
- **Drop** — how many octaves above Tune the pitch starts.
- **P.Hold** — how long the pitch sits at Drop before it falls. 0 is a one-stage fall, no pause at the top.
- **P.Decay** — how fast the pitch falls to Tune.
- **Click** — noise attack.
- **Attack / Punch** — amp attack time (0.3–50 ms). Short is clickier. The waveform draws an Ableton-style fade wedge for that length; it is a hint, not a clip-start trim.
- **A.Hold** — how long the amp sits at 1 after Punch, before Decay. 0 is attack-then-fall.
- **Decay** — amp tail.
- **Drive** — tanh saturation. Oversampled when Drive is up; 0 stays 1× tanh.
- **Harm** — extra 2nd harmonic plus a little 3rd. 707 and Hard still add their own 2nd on top. 0 is a sine body.
- **FM** — a little phase modulation of the body. 0 is off.
- **Vel** — how much MIDI velocity thins Click, Punch, Decay, and extra layer gains. 0 ignores velocity. Velocity 127 (and Play) matches the full sound.
- **Width** — a little stereo on Noise/Click layers above about 120 Hz. 0 is fully mono. The sub stays mono either way, as it should.
- **HPF** — rumble cut.
- **Tone** — low-pass.
- **Lo / Hi** — shelves (±12 dB around centre).
- **Style** — 909, 808, 707, Tight, Hard. Changes click, pitch/amp envelopes, and a bit of harmonic. Does not reset knobs.
- **Shape** — sine (default) or triangle body.
- **Length** — Free, or trim to 1/4, 1/8, 1/16 of host BPM (120 if the host has no tempo).
- **Hold** — One-shot (default) dies after the decay. Hold keeps the tail going until the next note. Length snaps still cut. Style 808 does not turn this on by itself.

**Volume** is the title-bar fader on the right, with a glance spectrum. It turns orange for about 800 ms if the output hits 0 dBFS. Play is clipped at 0 dBFS so it does not splat; Volume still flashes if it *would* have hit the ceiling.
