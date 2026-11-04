# DSP notes

- Body: sine (+ 2nd harmonic on 707/Hard). Start frequency = Tune × 2^Drop, exponential fall to Tune.
- Click: high-passed LCG noise. Style sets click time and gain (909 ≈ 4 ms).
- Noise layer: same LCG, then White / Pink (Kellet 3-pole) / Brown (leaky integrator). Pre-delay, optional attack fade, decay, and a length window (0 = Auto). HP then LP.
- Bass layer: sine at Tune×2^(semitones/12), optional Drop (octaves, same Decay as amp), tanh Drive, HP then LP, Length/Attack like Noise.
- Click layer: sine+noise mix, then HP/LP, Length/Attack like Noise.
- Sweep layer: falling sine (Start, Drop octaves, Decay). Optional tanh Drive. LP Tone. Length/Attack like Noise.
- Style also scales pitch-decay and amp-decay. Knobs stay independent.
- Attack / Punch maps to 0.3–50 ms amp attack, then exponential decay. Waveform overlay matches that time; it does not skip samples. The preview is the summed layers (post layer EQ, mute/solo/gain, master), before post FX.
- Drive: `tanh(x * (1 + drive * 4))`.
- Filters: one-pole HP then LP. Lo/Hi are ±12 dB shelves around 0.5.
- Length snap hard-gates with a 25 ms fade so the cut is not a click.
- Post FX sit after the voice: each layer has a Send (default 1) into Distortion → Delay (tempo sync optional) → Reverb pre-delay (ms, or Sync snaps to 1/64…1/8) → Reverb, all fully wet. Dist types: Soft `tanh(x * drive)`, Hard clip, Fold `sin(x * drive)`, Diode (weaker negative). Drive is `1 + fxDstDrive * 12`. Shape offsets then subtracts DC. Tone is a post LP (skipped when fully open). Dry and FX faders mix. Defaults Dry=1 FX=0, Soft, open Tone, Shape centre, pre-delay 0 so old patches are unchanged.
- Post EQ is five RBJ biquads after the mixer. Analyser FFT is after the EQ.
- Not a transistor model. If it sits off on a real clock/interface, Tune/Drop/Decay are the calibration knobs.
