#pragma once

namespace ParamID
{
constexpr const char* tune       = "tune";
constexpr const char* pitchDrop  = "pitchDrop";
constexpr const char* pitchDecay = "pitchDecay";
constexpr const char* click      = "click";
constexpr const char* punch      = "punch";
constexpr const char* decay      = "decay";
constexpr const char* drive      = "drive";
constexpr const char* hpf        = "hpf";
constexpr const char* tone       = "tone";
constexpr const char* lo         = "lo";
constexpr const char* hi         = "hi";
constexpr const char* level      = "level";
constexpr const char* length     = "length";
constexpr const char* style      = "style";

// Layer 0 is the kick. Slots 1..3 are optional voices. Kick has no type.
inline constexpr int layerCount = 4;
inline constexpr int slotCount = 3;
inline constexpr int layerOff = 0;
inline constexpr int layerBass = 1;
inline constexpr int layerNoise = 2;
inline constexpr int layerClick = 3;
inline constexpr int layerSweep = 4;

inline constexpr int layerOfSlot (int slot) { return slot + 1; }

inline constexpr const char* lyGain[layerCount] = { "ly0Gain", "ly1Gain", "ly2Gain", "ly3Gain" };
inline constexpr const char* lyMute[layerCount] = { "ly0Mute", "ly1Mute", "ly2Mute", "ly3Mute" };
inline constexpr const char* lySolo[layerCount] = { "ly0Solo", "ly1Solo", "ly2Solo", "ly3Solo" };
inline constexpr const char* lyFlip[layerCount] = { "ly0Flip", "ly1Flip", "ly2Flip", "ly3Flip" };
inline constexpr const char* lyDelay[layerCount] = { "ly0Delay", "ly1Delay", "ly2Delay", "ly3Delay" };

inline constexpr const char* lyType[slotCount] = { "ly1Type", "ly2Type", "ly3Type" };

inline constexpr const char* lyBassTune[slotCount]  = { "ly1BassTune", "ly2BassTune", "ly3BassTune" };
inline constexpr const char* lyBassDecay[slotCount] = { "ly1BassDecay", "ly2BassDecay", "ly3BassDecay" };
inline constexpr const char* lyBassDrive[slotCount] = { "ly1BassDrive", "ly2BassDrive", "ly3BassDrive" };
inline constexpr const char* lyBassHp[slotCount]    = { "ly1BassHp", "ly2BassHp", "ly3BassHp" };

inline constexpr const char* lyNoiseDecay[slotCount] = { "ly1NoiseDecay", "ly2NoiseDecay", "ly3NoiseDecay" };
inline constexpr const char* lyNoiseHp[slotCount]    = { "ly1NoiseHp", "ly2NoiseHp", "ly3NoiseHp" };
inline constexpr const char* lyNoiseLp[slotCount]    = { "ly1NoiseLp", "ly2NoiseLp", "ly3NoiseLp" };

inline constexpr const char* lyClickPitch[slotCount] = { "ly1ClickPitch", "ly2ClickPitch", "ly3ClickPitch" };
inline constexpr const char* lyClickDecay[slotCount] = { "ly1ClickDecay", "ly2ClickDecay", "ly3ClickDecay" };
inline constexpr const char* lyClickTone[slotCount]  = { "ly1ClickTone", "ly2ClickTone", "ly3ClickTone" };

inline constexpr const char* lySweepStart[slotCount] = { "ly1SweepStart", "ly2SweepStart", "ly3SweepStart" };
inline constexpr const char* lySweepDrop[slotCount]  = { "ly1SweepDrop", "ly2SweepDrop", "ly3SweepDrop" };
inline constexpr const char* lySweepDecay[slotCount] = { "ly1SweepDecay", "ly2SweepDecay", "ly3SweepDecay" };

constexpr const char* fxDry      = "fxDry";
constexpr const char* fxWet      = "fxWet";
constexpr const char* fxDstOn    = "fxDstOn";
constexpr const char* fxDstType  = "fxDstType";
constexpr const char* fxDstDrive = "fxDstDrive";
constexpr const char* fxDstTone  = "fxDstTone";
constexpr const char* fxDstShape = "fxDstShape";
constexpr const char* fxDlyOn    = "fxDlyOn";
constexpr const char* fxDlyTime  = "fxDlyTime";
constexpr const char* fxDlyFb    = "fxDlyFb";
constexpr const char* fxDlyCut   = "fxDlyCut";
constexpr const char* fxDlySync  = "fxDlySync";
constexpr const char* fxDlyDiv   = "fxDlyDiv";
constexpr const char* fxRevOn       = "fxRevOn";
constexpr const char* fxRevSize     = "fxRevSize";
constexpr const char* fxRevDamp     = "fxRevDamp";
constexpr const char* fxRevWidth    = "fxRevWidth";
constexpr const char* fxRevPredelay = "fxRevPredelay";
constexpr const char* fxRevSync     = "fxRevSync";
constexpr const char* fxRevDiv      = "fxRevDiv";

inline constexpr int eqBandCount = 5;
inline constexpr const char* postEqOn = "postEqOn";
inline constexpr const char* postEqType[eqBandCount] = {
    "postEq1Type", "postEq2Type", "postEq3Type", "postEq4Type", "postEq5Type"
};
inline constexpr const char* postEqFreq[eqBandCount] = {
    "postEq1Freq", "postEq2Freq", "postEq3Freq", "postEq4Freq", "postEq5Freq"
};
inline constexpr const char* postEqGain[eqBandCount] = {
    "postEq1Gain", "postEq2Gain", "postEq3Gain", "postEq4Gain", "postEq5Gain"
};
inline constexpr const char* postEqQ[eqBandCount] = {
    "postEq1Q", "postEq2Q", "postEq3Q", "postEq4Q", "postEq5Q"
};
}
