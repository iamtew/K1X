# DSP notes

- Body: sine. Start frequency = Tune × 2^Drop, exponential fall to Tune.
- Click: ~4 ms high-passed LCG noise.
- Punch maps to 0.3–12 ms amp attack, then exponential decay.
- Drive: `tanh(x * (1 + drive * 4))`.
- Filters: one-pole HP then LP. Lo/Hi are ±12 dB shelves around 0.5.
- Length snap hard-gates with a 5 ms fade so the cut is not a click.
- Not a transistor model. If it sits off on a real clock/interface, Tune/Drop/Decay are the calibration knobs.
