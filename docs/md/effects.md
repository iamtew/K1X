# Effects

The Effects page is a parallel send after the layered mix. **Dry** is the unprocessed mix. **FX** is Distortion → Delay → Reverb. Mix **Send** (per layer) is how much of that layer goes into the send. Old patches stay the same. Turn Dist / Delay / Rev on per block — nothing is secretly wet until you ask.

- **Dist** — Type (Soft tanh, Hard clip, Fold, Diode), Drive, Tone (post low-pass), Shape (asymmetry). Separate from the kick Drive knob. Runs 2× oversampled when Dist is on.
- **Delay** — Time, feedback, low-pass Cut. **Sync** uses host BPM and **Div** (1/16 … 1/2).
- **Reverb** — Predelay (ms), Size, Damp, Width. **Sync** snaps Predelay to 1/64 … 1/8 of host BPM.

Post EQ sits under the FX row on the same page. See [eq.md](eq.md).
