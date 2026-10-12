# Architecture

PluginProcessor owns APVTS and one KickVoice.

Audio thread:

1. MIDI note-on or UI Play sets a trigger
2. MIDI note sets body Hz; Play uses the Tune knob. Voice also reads style and length (host BPM)
3. Click + sine → tanh → HP → LP → lo/hi → length gate → stereo copy

Message thread:

- KickRender fills a buffer for the waveform panel and WAV export
- Same KickVoice as the audio path
