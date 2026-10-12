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

constexpr const char* fxDry      = "fxDry";
constexpr const char* fxWet      = "fxWet";
constexpr const char* fxDstOn    = "fxDstOn";
constexpr const char* fxDstDrive = "fxDstDrive";
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
