# Architecture

PluginProcessor owns APVTS and one KickVoice.

Audio thread:

1. MIDI note-on or UI Play sets a trigger
2. Voice reads params (style recipe, length from host BPM)
3. Click + sine → tanh → HP → LP → lo/hi → length gate → stereo copy

Message thread:

- KickRender fills a buffer for the waveform panel and WAV export
- Same KickVoice as the audio path
