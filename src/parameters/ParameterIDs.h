#pragma once

namespace ParamID
{
inline constexpr int eqBandCount = 5;

constexpr const char* tune       = "tune";
constexpr const char* pitchDrop  = "pitchDrop";
constexpr const char* pitchHold  = "pitchHold";
constexpr const char* pitchDecay = "pitchDecay";
constexpr const char* click      = "click";
constexpr const char* punch      = "punch";
constexpr const char* ampHold    = "ampHold";
constexpr const char* decay      = "decay";
constexpr const char* drive      = "drive";
constexpr const char* hpf        = "hpf";
constexpr const char* tone       = "tone";
constexpr const char* lo         = "lo";
constexpr const char* hi         = "hi";
constexpr const char* level      = "level";
constexpr const char* length     = "length";
constexpr const char* style      = "style";
constexpr const char* bodyShape  = "bodyShape";
constexpr const char* harm       = "harm";
constexpr const char* fm         = "fm";
constexpr const char* velAmt     = "velAmt";

// Layer 0 is the kick. Slots 1..3 are optional voices. Kick has no type.
inline constexpr int layerCount = 4;
inline constexpr int slotCount = 3;
inline constexpr int layerOff = 0;
inline constexpr int layerBass = 1;
inline constexpr int layerNoise = 2;
inline constexpr int layerClick = 3;
inline constexpr int layerSweep = 4;
inline constexpr int layerSample = 5;

inline constexpr int layerOfSlot (int slot) { return slot + 1; }

inline constexpr const char* lyGain[layerCount] = { "ly0Gain", "ly1Gain", "ly2Gain", "ly3Gain" };
inline constexpr const char* lyMute[layerCount] = { "ly0Mute", "ly1Mute", "ly2Mute", "ly3Mute" };
inline constexpr const char* lySolo[layerCount] = { "ly0Solo", "ly1Solo", "ly2Solo", "ly3Solo" };
inline constexpr const char* lyFlip[layerCount] = { "ly0Flip", "ly1Flip", "ly2Flip", "ly3Flip" };
inline constexpr const char* lySend[layerCount] = { "ly0Send", "ly1Send", "ly2Send", "ly3Send" };
inline constexpr const char* lyDelay[layerCount] = { "ly0Delay", "ly1Delay", "ly2Delay", "ly3Delay" };

inline constexpr const char* lyType[slotCount] = { "ly1Type", "ly2Type", "ly3Type" };

inline constexpr const char* lyBassTune[slotCount]  = { "ly1BassTune", "ly2BassTune", "ly3BassTune" };
inline constexpr const char* lyBassDrop[slotCount]  = { "ly1BassDrop", "ly2BassDrop", "ly3BassDrop" };
inline constexpr const char* lyBassDecay[slotCount] = { "ly1BassDecay", "ly2BassDecay", "ly3BassDecay" };
inline constexpr const char* lyBassDrive[slotCount] = { "ly1BassDrive", "ly2BassDrive", "ly3BassDrive" };
inline constexpr const char* lyBassHp[slotCount]    = { "ly1BassHp", "ly2BassHp", "ly3BassHp" };
inline constexpr const char* lyBassLp[slotCount]    = { "ly1BassLp", "ly2BassLp", "ly3BassLp" };
inline constexpr const char* lyBassLen[slotCount]   = { "ly1BassLen", "ly2BassLen", "ly3BassLen" };
inline constexpr const char* lyBassAtk[slotCount]   = { "ly1BassAtk", "ly2BassAtk", "ly3BassAtk" };

inline constexpr const char* lyNoiseDecay[slotCount] = { "ly1NoiseDecay", "ly2NoiseDecay", "ly3NoiseDecay" };
inline constexpr const char* lyNoiseHp[slotCount]    = { "ly1NoiseHp", "ly2NoiseHp", "ly3NoiseHp" };
inline constexpr const char* lyNoiseLp[slotCount]    = { "ly1NoiseLp", "ly2NoiseLp", "ly3NoiseLp" };
inline constexpr const char* lyNoiseType[slotCount]  = { "ly1NoiseType", "ly2NoiseType", "ly3NoiseType" };
inline constexpr const char* lyNoiseLen[slotCount]   = { "ly1NoiseLen", "ly2NoiseLen", "ly3NoiseLen" };
inline constexpr const char* lyNoiseAtk[slotCount]   = { "ly1NoiseAtk", "ly2NoiseAtk", "ly3NoiseAtk" };

inline constexpr int noiseWhite = 0;
inline constexpr int noisePink = 1;
inline constexpr int noiseBrown = 2;

inline constexpr const char* lyClickPitch[slotCount] = { "ly1ClickPitch", "ly2ClickPitch", "ly3ClickPitch" };
inline constexpr const char* lyClickDecay[slotCount] = { "ly1ClickDecay", "ly2ClickDecay", "ly3ClickDecay" };
inline constexpr const char* lyClickTone[slotCount]  = { "ly1ClickTone", "ly2ClickTone", "ly3ClickTone" };
inline constexpr const char* lyClickHp[slotCount]    = { "ly1ClickHp", "ly2ClickHp", "ly3ClickHp" };
inline constexpr const char* lyClickLp[slotCount]    = { "ly1ClickLp", "ly2ClickLp", "ly3ClickLp" };
inline constexpr const char* lyClickLen[slotCount]   = { "ly1ClickLen", "ly2ClickLen", "ly3ClickLen" };
inline constexpr const char* lyClickAtk[slotCount]   = { "ly1ClickAtk", "ly2ClickAtk", "ly3ClickAtk" };

inline constexpr const char* lySweepStart[slotCount] = { "ly1SweepStart", "ly2SweepStart", "ly3SweepStart" };
inline constexpr const char* lySweepDrop[slotCount]  = { "ly1SweepDrop", "ly2SweepDrop", "ly3SweepDrop" };
inline constexpr const char* lySweepDecay[slotCount] = { "ly1SweepDecay", "ly2SweepDecay", "ly3SweepDecay" };
inline constexpr const char* lySweepDrive[slotCount] = { "ly1SweepDrive", "ly2SweepDrive", "ly3SweepDrive" };
inline constexpr const char* lySweepLp[slotCount]    = { "ly1SweepLp", "ly2SweepLp", "ly3SweepLp" };
inline constexpr const char* lySweepLen[slotCount]   = { "ly1SweepLen", "ly2SweepLen", "ly3SweepLen" };
inline constexpr const char* lySweepAtk[slotCount]   = { "ly1SweepAtk", "ly2SweepAtk", "ly3SweepAtk" };

inline constexpr const char* lySampleHp[slotCount]  = { "ly1SampleHp", "ly2SampleHp", "ly3SampleHp" };
inline constexpr const char* lySampleLp[slotCount]  = { "ly1SampleLp", "ly2SampleLp", "ly3SampleLp" };
inline constexpr const char* lySampleLen[slotCount] = { "ly1SampleLen", "ly2SampleLen", "ly3SampleLen" };

inline constexpr const char* lyEqOn[layerCount] = { "ly0EqOn", "ly1EqOn", "ly2EqOn", "ly3EqOn" };
inline constexpr const char* lyEqType[layerCount][eqBandCount] = {
    { "ly0Eq1Type", "ly0Eq2Type", "ly0Eq3Type", "ly0Eq4Type", "ly0Eq5Type" },
    { "ly1Eq1Type", "ly1Eq2Type", "ly1Eq3Type", "ly1Eq4Type", "ly1Eq5Type" },
    { "ly2Eq1Type", "ly2Eq2Type", "ly2Eq3Type", "ly2Eq4Type", "ly2Eq5Type" },
    { "ly3Eq1Type", "ly3Eq2Type", "ly3Eq3Type", "ly3Eq4Type", "ly3Eq5Type" }
};
inline constexpr const char* lyEqFreq[layerCount][eqBandCount] = {
    { "ly0Eq1Freq", "ly0Eq2Freq", "ly0Eq3Freq", "ly0Eq4Freq", "ly0Eq5Freq" },
    { "ly1Eq1Freq", "ly1Eq2Freq", "ly1Eq3Freq", "ly1Eq4Freq", "ly1Eq5Freq" },
    { "ly2Eq1Freq", "ly2Eq2Freq", "ly2Eq3Freq", "ly2Eq4Freq", "ly2Eq5Freq" },
    { "ly3Eq1Freq", "ly3Eq2Freq", "ly3Eq3Freq", "ly3Eq4Freq", "ly3Eq5Freq" }
};
inline constexpr const char* lyEqGain[layerCount][eqBandCount] = {
    { "ly0Eq1Gain", "ly0Eq2Gain", "ly0Eq3Gain", "ly0Eq4Gain", "ly0Eq5Gain" },
    { "ly1Eq1Gain", "ly1Eq2Gain", "ly1Eq3Gain", "ly1Eq4Gain", "ly1Eq5Gain" },
    { "ly2Eq1Gain", "ly2Eq2Gain", "ly2Eq3Gain", "ly2Eq4Gain", "ly2Eq5Gain" },
    { "ly3Eq1Gain", "ly3Eq2Gain", "ly3Eq3Gain", "ly3Eq4Gain", "ly3Eq5Gain" }
};
inline constexpr const char* lyEqQ[layerCount][eqBandCount] = {
    { "ly0Eq1Q", "ly0Eq2Q", "ly0Eq3Q", "ly0Eq4Q", "ly0Eq5Q" },
    { "ly1Eq1Q", "ly1Eq2Q", "ly1Eq3Q", "ly1Eq4Q", "ly1Eq5Q" },
    { "ly2Eq1Q", "ly2Eq2Q", "ly2Eq3Q", "ly2Eq4Q", "ly2Eq5Q" },
    { "ly3Eq1Q", "ly3Eq2Q", "ly3Eq3Q", "ly3Eq4Q", "ly3Eq5Q" }
};

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
