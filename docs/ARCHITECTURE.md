# Architecture

PluginProcessor owns APVTS and one KickVoice.

Audio thread:

1. MIDI note-on or UI Play sets a trigger. Loop + Play retrigs 4-on-the-floor; optional hats/bed mix after post EQ on the main bus only.
2. MIDI note sets body Hz; Play uses the Tune knob. Voice also reads style and length (host BPM)
3. Click + sine → tanh → HP → LP → lo/hi → length gate → stereo copy
4. Per-layer Send into Distortion → Delay → Reverb pre-delay → Reverb, mix Dry + FX, then 5-band post EQ (main bus only). Optional Kick / Slot 1–3 buses copy that layer before Width/FX.
5. EqAnalyser + meter tap the post-EQ buffer (spectrum shows the affected signal)

Message thread:

- Waveform panel uses the same layered mix as Play (post layer EQ, before post FX)
- WAV export runs the same post path plus ~2 s of tail
- Same KickVoice as the audio path
