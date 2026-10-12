# Architecture

PluginProcessor owns APVTS and one KickVoice.

Audio thread:

1. MIDI note-on or UI Play sets a trigger
2. MIDI note sets body Hz; Play uses the Tune knob. Voice also reads style and length (host BPM)
3. Click + sine → tanh → HP → LP → lo/hi → length gate → stereo copy
4. Parallel send: Distortion → Delay → Reverb, mix Dry + FX, then 5-band post EQ
5. EqAnalyser + meter tap the post-EQ buffer (spectrum shows the affected signal)

Message thread:

- KickRender fills a buffer for the waveform panel (dry kick)
- WAV export runs the same post path plus ~2 s of tail
- Same KickVoice as the audio path
