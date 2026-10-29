#pragma once

// Factory kicks. initPatch() runs first, then these voice values, then a
// matching FactoryFx / FactoryEq row if the name is listed there.
// Categories stay in one contiguous run — the patch menu opens one submenu each.
// Style: 0=909 1=808 2=707 3=Tight 4=Hard. Length: 0=Free 1=1/4 2=1/8 3=1/16.

#include <cstddef>

namespace k1x
{

struct FactoryPreset
{
    const char* category;
    const char* name;
    float tune, drop, pDec, click, punch, decay, drive, hpf, tone, lo, hi, level;
    int style, length;
};

// dst < 0 = distortion off. dlyDiv < 0 = delay off (else 0..3 = 1/16..1/2, sync stays on).
// rev < 0 = reverb off (else room size).
struct FactoryFx
{
    const char* name;
    float dry, wet, dst;
    int dlyDiv;
    float dlyFb;
    float rev, revDamp;
};

// type2 == 0 leaves band 2 at init. Types: 1 peak, 2 low shelf, 3 high shelf, 4 HP, 5 LP.
struct FactoryEq
{
    const char* name;
    int type;
    float hz, db, q;
    int type2;
    float hz2, db2, q2;
};

inline constexpr FactoryPreset factoryPresets[] = {
    // 909
    { "909", "Punch",     51.f, 1.6f, 0.030f, 0.62f, 0.12f, 0.32f, 0.35f,  28.f, 4500.f, 0.60f, 0.52f, 0.72f, 0, 0 },
    { "909", "Short",     58.f, 1.2f, 0.020f, 0.50f, 0.08f, 0.12f, 0.22f,  35.f, 5000.f, 0.48f, 0.55f, 0.74f, 0, 0 },
    { "909", "Long",      46.f, 1.4f, 0.060f, 0.35f, 0.30f, 0.85f, 0.18f,  22.f, 3200.f, 0.66f, 0.40f, 0.73f, 0, 0 },
    { "909", "Click",     62.f, 2.2f, 0.015f, 0.90f, 0.05f, 0.22f, 0.30f,  40.f, 6500.f, 0.42f, 0.70f, 0.70f, 0, 0 },
    { "909", "Soft",      48.f, 0.8f, 0.070f, 0.15f, 0.55f, 0.50f, 0.08f,  20.f, 2200.f, 0.58f, 0.32f, 0.76f, 0, 0 },
    { "909", "Rolling",   50.f, 1.1f, 0.045f, 0.28f, 0.35f, 0.42f, 0.15f,  26.f, 2800.f, 0.60f, 0.38f, 0.75f, 0, 0 },
    // 808
    { "808", "Fat",       42.f, 0.9f, 0.050f, 0.20f, 0.40f, 0.70f, 0.25f,  18.f, 1800.f, 0.78f, 0.30f, 0.70f, 1, 0 },
    { "808", "Sub",       34.f, 0.6f, 0.080f, 0.08f, 0.50f, 0.90f, 0.12f,  12.f,  900.f, 0.85f, 0.22f, 0.72f, 1, 0 },
    { "808", "Long Tail", 38.f, 0.7f, 0.100f, 0.10f, 0.45f, 1.60f, 0.10f,  14.f, 1200.f, 0.80f, 0.25f, 0.68f, 1, 0 },
    { "808", "Trap Sub",  36.f, 1.3f, 0.030f, 0.35f, 0.15f, 0.55f, 0.40f,  16.f, 2400.f, 0.82f, 0.40f, 0.66f, 1, 0 },
    { "808", "Bounce",    55.f, 1.0f, 0.040f, 0.25f, 0.20f, 0.28f, 0.30f,  30.f, 3000.f, 0.70f, 0.42f, 0.73f, 1, 0 },
    { "808", "Boom",      40.f, 1.8f, 0.090f, 0.18f, 0.35f, 1.10f, 0.22f,  15.f, 1500.f, 0.84f, 0.28f, 0.68f, 1, 0 },
    // 707
    { "707", "Dry",       56.f, 1.0f, 0.025f, 0.45f, 0.10f, 0.40f, 0.10f,  32.f, 5500.f, 0.45f, 0.55f, 0.75f, 2, 0 },
    { "707", "Snap",      64.f, 1.6f, 0.018f, 0.75f, 0.05f, 0.25f, 0.20f,  45.f, 7000.f, 0.40f, 0.68f, 0.72f, 2, 0 },
    { "707", "Electro",   50.f, 2.0f, 0.040f, 0.55f, 0.12f, 0.35f, 0.35f,  28.f, 4800.f, 0.50f, 0.60f, 0.70f, 2, 0 },
    { "707", "Thin",      70.f, 0.8f, 0.020f, 0.60f, 0.08f, 0.20f, 0.15f,  90.f, 6000.f, 0.28f, 0.72f, 0.74f, 2, 0 },
    { "707", "Metallic",  74.f, 2.4f, 0.030f, 0.85f, 0.06f, 0.30f, 0.45f,  70.f, 7500.f, 0.32f, 0.80f, 0.66f, 2, 0 },
    { "707", "Tick",      78.f, 0.4f, 0.012f, 0.95f, 0.02f, 0.12f, 0.12f, 120.f, 7800.f, 0.25f, 0.75f, 0.74f, 2, 0 },
    // EDM
    { "EDM", "Festival",    48.f, 1.9f, 0.028f, 0.70f, 0.10f, 0.38f, 0.45f, 30.f, 5000.f, 0.68f, 0.58f, 0.66f, 0, 0 },
    { "EDM", "Big Room",    50.f, 2.2f, 0.022f, 0.80f, 0.08f, 0.30f, 0.55f, 34.f, 4200.f, 0.64f, 0.50f, 0.62f, 0, 0 },
    { "EDM", "Future",      46.f, 1.5f, 0.035f, 0.40f, 0.22f, 0.45f, 0.30f, 24.f, 3000.f, 0.72f, 0.40f, 0.70f, 3, 0 },
    { "EDM", "House",       53.f, 1.2f, 0.040f, 0.48f, 0.18f, 0.34f, 0.22f, 32.f, 3800.f, 0.58f, 0.48f, 0.74f, 0, 0 },
    { "EDM", "Trance",      55.f, 1.4f, 0.050f, 0.35f, 0.28f, 0.55f, 0.18f, 28.f, 3400.f, 0.60f, 0.42f, 0.74f, 0, 0 },
    { "EDM", "Progressive", 49.f, 1.0f, 0.060f, 0.30f, 0.32f, 0.48f, 0.15f, 26.f, 2600.f, 0.62f, 0.36f, 0.75f, 0, 0 },
    // Techno
    { "Techno", "Warehouse",  51.f, 1.50f, 0.032f, 0.58f, 0.14f, 0.30f, 0.32f, 36.f, 3600.f, 0.55f, 0.48f, 0.72f, 0, 0 },
    { "Techno", "Dub",        47.f, 1.10f, 0.055f, 0.22f, 0.40f, 0.60f, 0.12f, 22.f, 2000.f, 0.70f, 0.30f, 0.74f, 0, 0 },
    { "Techno", "Hypnotic",   49.f, 0.70f, 0.080f, 0.18f, 0.45f, 0.40f, 0.10f, 24.f, 1800.f, 0.64f, 0.28f, 0.76f, 0, 0 },
    { "Techno", "Peak Time",  54.f, 1.80f, 0.025f, 0.72f, 0.10f, 0.26f, 0.40f, 38.f, 4800.f, 0.52f, 0.58f, 0.68f, 0, 0 },
    { "Techno", "Industrial", 44.f, 1.30f, 0.040f, 0.50f, 0.20f, 0.36f, 0.65f, 40.f, 2500.f, 0.58f, 0.35f, 0.60f, 4, 0 },
    { "Techno", "Berlin",     52.f, 1.35f, 0.038f, 0.42f, 0.22f, 0.33f, 0.25f, 33.f, 3200.f, 0.57f, 0.44f, 0.73f, 0, 0 },
    // Hardcore
    { "Hardcore", "Gabber",     58.f, 2.4f, 0.018f, 0.85f, 0.04f, 0.16f, 0.75f, 45.f, 5500.f, 0.50f, 0.62f, 0.55f, 4, 0 },
    { "Hardcore", "Frenchcore", 62.f, 2.6f, 0.015f, 0.95f, 0.02f, 0.12f, 0.85f, 55.f, 6500.f, 0.42f, 0.75f, 0.52f, 4, 0 },
    { "Hardcore", "Terror",     48.f, 2.8f, 0.020f, 0.70f, 0.06f, 0.20f, 0.95f, 30.f, 4000.f, 0.66f, 0.40f, 0.48f, 4, 0 },
    { "Hardcore", "Uptempo",    68.f, 2.0f, 0.012f, 0.90f, 0.03f, 0.10f, 0.60f, 60.f, 7000.f, 0.38f, 0.70f, 0.60f, 4, 0 },
    { "Hardcore", "Hardstyle",  50.f, 2.2f, 0.030f, 0.55f, 0.12f, 0.45f, 0.50f, 28.f, 3800.f, 0.72f, 0.45f, 0.62f, 4, 0 },
    { "Hardcore", "Rawstyle",   46.f, 2.5f, 0.022f, 0.65f, 0.08f, 0.28f, 0.80f, 32.f, 3200.f, 0.70f, 0.38f, 0.55f, 4, 0 },
    // Distorted
    { "Distorted", "Fuzz",       45.f, 1.6f, 0.040f, 0.40f, 0.20f, 0.40f, 0.70f, 25.f, 2200.f, 0.65f, 0.35f, 0.55f, 4, 0 },
    { "Distorted", "Clip",       52.f, 1.2f, 0.020f, 0.60f, 0.05f, 0.22f, 0.90f, 35.f, 5000.f, 0.50f, 0.55f, 0.50f, 4, 0 },
    { "Distorted", "Saturation", 48.f, 1.4f, 0.045f, 0.30f, 0.25f, 0.50f, 0.45f, 22.f, 2800.f, 0.68f, 0.40f, 0.64f, 4, 0 },
    { "Distorted", "Chainsaw",   60.f, 2.2f, 0.015f, 0.80f, 0.04f, 0.18f, 0.85f, 80.f, 6000.f, 0.35f, 0.80f, 0.50f, 4, 0 },
    { "Distorted", "Cone",       40.f, 1.0f, 0.070f, 0.20f, 0.35f, 0.60f, 0.75f, 18.f,  900.f, 0.80f, 0.20f, 0.58f, 4, 0 },
    { "Distorted", "Overdrive",  54.f, 1.5f, 0.030f, 0.50f, 0.15f, 0.32f, 0.55f, 30.f, 3600.f, 0.58f, 0.50f, 0.62f, 4, 0 },
    // Hip-Hop
    { "Hip-Hop", "Boom Bap",    56.f, 1.3f, 0.035f, 0.55f, 0.18f, 0.28f, 0.28f, 40.f, 3400.f, 0.52f, 0.46f, 0.73f, 0, 0 },
    { "Hip-Hop", "Lo-Fi",       44.f, 0.8f, 0.060f, 0.15f, 0.50f, 0.45f, 0.35f, 20.f, 1200.f, 0.70f, 0.22f, 0.72f, 1, 0 },
    { "Hip-Hop", "Drill",       40.f, 1.6f, 0.025f, 0.45f, 0.10f, 0.35f, 0.50f, 22.f, 2600.f, 0.78f, 0.42f, 0.64f, 1, 0 },
    { "Hip-Hop", "West Coast",  38.f, 0.5f, 0.090f, 0.12f, 0.55f, 0.85f, 0.18f, 16.f, 1400.f, 0.80f, 0.28f, 0.70f, 1, 0 },
    { "Hip-Hop", "Knock",       53.f, 1.7f, 0.028f, 0.68f, 0.10f, 0.24f, 0.38f, 36.f, 4200.f, 0.60f, 0.52f, 0.70f, 0, 0 },
    { "Hip-Hop", "Modern",      43.f, 1.2f, 0.040f, 0.30f, 0.22f, 0.50f, 0.32f, 20.f, 2000.f, 0.75f, 0.36f, 0.70f, 1, 0 },
    // Punch
    { "Punch", "Knockout",  50.f, 2.0f, 0.020f, 0.75f, 0.05f, 0.26f, 0.42f,  32.f, 4800.f, 0.62f, 0.55f, 0.68f, 0, 0 },
    { "Punch", "Smack",     60.f, 1.4f, 0.018f, 0.85f, 0.04f, 0.30f, 0.35f,  48.f, 6200.f, 0.40f, 0.66f, 0.70f, 3, 0 },
    { "Punch", "Finger",    72.f, 0.6f, 0.012f, 0.95f, 0.02f, 0.18f, 0.15f, 100.f, 7500.f, 0.30f, 0.72f, 0.74f, 2, 0 },
    { "Punch", "Transient", 55.f, 2.6f, 0.012f, 0.88f, 0.02f, 0.20f, 0.25f,  42.f, 7000.f, 0.45f, 0.70f, 0.70f, 0, 0 },
    { "Punch", "Dry Hit",   52.f, 1.1f, 0.030f, 0.40f, 0.12f, 0.22f, 0.05f,  30.f, 4000.f, 0.50f, 0.50f, 0.76f, 0, 0 },
    { "Punch", "Gated",     58.f, 1.3f, 0.020f, 0.60f, 0.08f, 0.40f, 0.30f,  40.f, 5000.f, 0.48f, 0.55f, 0.72f, 3, 3 },
    // Weird
    { "Weird", "Laser",  70.f, 2.9f, 0.120f, 0.20f, 0.15f, 0.45f, 0.20f, 40.f, 6000.f, 0.35f, 0.65f, 0.70f, 0, 0 },
    { "Weird", "Hollow", 36.f, 0.4f, 0.150f, 0.05f, 0.60f, 0.80f, 0.05f, 50.f,  600.f, 0.40f, 0.20f, 0.74f, 1, 0 },
    { "Weird", "Rubber", 42.f, 1.2f, 0.180f, 0.10f, 0.70f, 0.55f, 0.15f, 20.f, 1500.f, 0.75f, 0.25f, 0.72f, 1, 0 },
    { "Weird", "Alien",  66.f, 2.7f, 0.080f, 0.70f, 0.10f, 0.35f, 0.60f, 90.f, 4500.f, 0.30f, 0.78f, 0.60f, 4, 0 },
    { "Weird", "Water",  40.f, 0.9f, 0.100f, 0.12f, 0.65f, 0.70f, 0.08f, 18.f, 1600.f, 0.72f, 0.30f, 0.74f, 0, 0 },
    { "Weird", "Zap",    76.f, 3.0f, 0.010f, 1.00f, 0.00f, 0.15f, 0.40f, 60.f, 8000.f, 0.28f, 0.85f, 0.62f, 2, 0 },
    // Tight — Clicky, not Click: that name is the 909.
    { "Tight", "Sixteenth", 54.f, 1.2f, 0.020f, 0.55f, 0.10f, 0.25f, 0.20f,  35.f, 4500.f, 0.50f, 0.50f, 0.74f, 3, 3 },
    { "Tight", "Eighth",    48.f, 1.0f, 0.035f, 0.40f, 0.20f, 0.40f, 0.18f,  28.f, 3200.f, 0.58f, 0.42f, 0.74f, 3, 2 },
    { "Tight", "Clicky",    68.f, 1.8f, 0.015f, 0.90f, 0.04f, 0.20f, 0.22f,  55.f, 6800.f, 0.35f, 0.70f, 0.72f, 3, 3 },
    { "Tight", "Minimal",   50.f, 0.5f, 0.040f, 0.20f, 0.30f, 0.18f, 0.08f,  30.f, 2400.f, 0.52f, 0.35f, 0.76f, 3, 0 },
    { "Tight", "Perc",      74.f, 0.7f, 0.012f, 0.80f, 0.05f, 0.15f, 0.12f, 110.f, 7500.f, 0.25f, 0.68f, 0.74f, 3, 3 },
    { "Tight", "Grid",      52.f, 1.4f, 0.025f, 0.50f, 0.12f, 0.30f, 0.25f,  32.f, 4000.f, 0.55f, 0.48f, 0.73f, 3, 2 },
    // Hard
    { "Hard", "Stomp",      46.f, 1.8f, 0.030f, 0.60f, 0.12f, 0.40f, 0.55f, 26.f, 2800.f, 0.70f, 0.40f, 0.64f, 4, 0 },
    { "Hard", "Rock",       58.f, 1.2f, 0.025f, 0.70f, 0.10f, 0.28f, 0.40f, 45.f, 4200.f, 0.48f, 0.55f, 0.68f, 4, 0 },
    { "Hard", "Metal",      62.f, 1.6f, 0.018f, 0.88f, 0.05f, 0.18f, 0.65f, 55.f, 6000.f, 0.40f, 0.70f, 0.60f, 4, 0 },
    { "Hard", "Compressed", 50.f, 1.0f, 0.040f, 0.35f, 0.35f, 0.22f, 0.70f, 34.f, 2500.f, 0.60f, 0.42f, 0.58f, 4, 0 },
    { "Hard", "Aggressive", 54.f, 2.3f, 0.020f, 0.80f, 0.06f, 0.24f, 0.80f, 38.f, 5000.f, 0.55f, 0.60f, 0.55f, 4, 0 },
    { "Hard", "Live",       49.f, 0.9f, 0.050f, 0.45f, 0.28f, 0.55f, 0.30f, 30.f, 3600.f, 0.58f, 0.48f, 0.70f, 4, 0 },
};

inline constexpr FactoryFx factoryFx[] = {
    { "Dub",        1.00f, 0.35f, -1.f,  2, 0.40f, -1.f,  0.40f },
    { "Industrial", 0.85f, 0.55f, 0.70f, -1, 0.35f, -1.f,  0.40f },
    { "Gabber",     0.70f, 0.80f, 0.85f, -1, 0.35f, -1.f,  0.40f },
    { "Frenchcore", 0.55f, 0.90f, 0.95f, -1, 0.35f, -1.f,  0.40f },
    { "Terror",     0.60f, 0.85f, 0.90f, -1, 0.35f, -1.f,  0.40f },
    { "Rawstyle",   0.65f, 0.75f, 0.80f, -1, 0.35f, -1.f,  0.40f },
    { "Fuzz",       0.50f, 1.00f, 0.75f, -1, 0.35f, -1.f,  0.40f },
    { "Clip",       0.40f, 1.00f, 0.95f, -1, 0.35f, -1.f,  0.40f },
    { "Saturation", 0.80f, 0.45f, 0.45f, -1, 0.35f, -1.f,  0.40f },
    { "Chainsaw",   0.45f, 0.90f, 0.85f, -1, 0.35f, -1.f,  0.40f },
    { "Cone",       0.55f, 0.80f, 0.70f, -1, 0.35f, -1.f,  0.40f },
    { "Overdrive",  0.75f, 0.50f, 0.55f, -1, 0.35f, -1.f,  0.40f },
    { "Laser",      1.00f, 0.40f, -1.f,  0, 0.50f, -1.f,  0.40f },
    { "Hollow",     0.90f, 0.45f, -1.f, -1, 0.35f, 0.70f, 0.60f },
    { "Alien",      0.80f, 0.55f, -1.f,  1, 0.35f, 0.55f, 0.35f },
    { "Water",      0.85f, 0.50f, -1.f, -1, 0.35f, 0.65f, 0.50f },
    { "Zap",        1.00f, 0.35f, -1.f,  0, 0.25f, -1.f,  0.40f },
    { "Live",       1.00f, 0.22f, -1.f, -1, 0.35f, 0.35f, 0.55f },
};

inline constexpr FactoryEq factoryEq[] = {
    { "Sub",       2,  55.f,  5.f, 0.70f, 1, 280.f, -3.f, 1.2f },
    { "Trap Sub",  2,  48.f,  6.f, 0.70f, 1, 350.f, -4.f, 1.2f },
    { "Hardstyle", 1, 320.f, -6.f, 1.40f, 3, 4000.f, 3.f, 0.7f },
    { "Hollow",    1, 180.f, -10.f, 0.60f, 5, 700.f, 0.f, 0.7f },
};

inline constexpr int factoryPresetCount = (int) (sizeof factoryPresets / sizeof factoryPresets[0]);
inline constexpr int factoryFxCount = (int) (sizeof factoryFx / sizeof factoryFx[0]);
inline constexpr int factoryEqCount = (int) (sizeof factoryEq / sizeof factoryEq[0]);

constexpr bool factorySame (const char* a, const char* b)
{
    if (a == nullptr || b == nullptr)
        return false;
    while (*a != '\0' && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

constexpr const FactoryPreset* findFactoryPreset (const char* name)
{
    for (int i = 0; i < factoryPresetCount; ++i)
        if (factorySame (factoryPresets[i].name, name))
            return &factoryPresets[i];
    return nullptr;
}

constexpr const FactoryFx* findFactoryFx (const char* name)
{
    for (int i = 0; i < factoryFxCount; ++i)
        if (factorySame (factoryFx[i].name, name))
            return &factoryFx[i];
    return nullptr;
}

constexpr const FactoryEq* findFactoryEq (const char* name)
{
    for (int i = 0; i < factoryEqCount; ++i)
        if (factorySame (factoryEq[i].name, name))
            return &factoryEq[i];
    return nullptr;
}

constexpr bool factoryInRange (float v, float lo, float hi)
{
    return v >= lo && v <= hi;
}

// Names unique, categories one run each, hints point at a preset, knobs inside the layout.
constexpr bool factoryBankOk()
{
    for (int i = 0; i < factoryPresetCount; ++i)
    {
        const FactoryPreset& p = factoryPresets[i];
        if (p.category == nullptr || p.category[0] == '\0' || p.name == nullptr || p.name[0] == '\0')
            return false;
        for (int j = 0; j < i; ++j)
            if (factorySame (p.name, factoryPresets[j].name))
                return false;
        if (i > 0 && ! factorySame (p.category, factoryPresets[i - 1].category))
            for (int j = 0; j < i; ++j)
                if (factorySame (p.category, factoryPresets[j].category))
                    return false;
        if (! factoryInRange (p.tune, 30.f, 80.f) || ! factoryInRange (p.drop, 0.f, 3.f)
            || ! factoryInRange (p.pDec, 0.01f, 0.2f) || ! factoryInRange (p.click, 0.f, 1.f)
            || ! factoryInRange (p.punch, 0.f, 1.f) || ! factoryInRange (p.decay, 0.05f, 2.f)
            || ! factoryInRange (p.drive, 0.f, 1.f) || ! factoryInRange (p.hpf, 10.f, 200.f)
            || ! factoryInRange (p.tone, 200.f, 8000.f) || ! factoryInRange (p.lo, 0.f, 1.f)
            || ! factoryInRange (p.hi, 0.f, 1.f) || ! factoryInRange (p.level, 0.f, 1.f)
            || p.style < 0 || p.style > 4 || p.length < 0 || p.length > 3)
            return false;
    }
    for (int i = 0; i < factoryFxCount; ++i)
    {
        const FactoryFx& f = factoryFx[i];
        if (findFactoryPreset (f.name) == nullptr)
            return false;
        for (int j = 0; j < i; ++j)
            if (factorySame (f.name, factoryFx[j].name))
                return false;
        if (! factoryInRange (f.dry, 0.f, 1.f) || ! factoryInRange (f.wet, 0.f, 1.f))
            return false;
        if (f.dst < 0.f)
        {
            if (f.dst != -1.f)
                return false;
        }
        else if (! factoryInRange (f.dst, 0.f, 1.f))
            return false;
        if (f.dlyDiv < 0)
        {
            if (f.dlyDiv != -1)
                return false;
        }
        else if (f.dlyDiv > 3 || ! factoryInRange (f.dlyFb, 0.f, 1.f))
            return false;
        if (f.rev < 0.f)
        {
            if (f.rev != -1.f)
                return false;
        }
        else if (! factoryInRange (f.rev, 0.f, 1.f) || ! factoryInRange (f.revDamp, 0.f, 1.f))
            return false;
    }
    for (int i = 0; i < factoryEqCount; ++i)
    {
        const FactoryEq& e = factoryEq[i];
        if (findFactoryPreset (e.name) == nullptr)
            return false;
        for (int j = 0; j < i; ++j)
            if (factorySame (e.name, factoryEq[j].name))
                return false;
        if (e.type < 1 || e.type > 5 || ! factoryInRange (e.hz, 20.f, 20000.f)
            || ! factoryInRange (e.db, -12.f, 12.f) || ! factoryInRange (e.q, 0.1f, 10.f))
            return false;
        if (e.type2 == 0)
            continue;
        if (e.type2 < 1 || e.type2 > 5 || ! factoryInRange (e.hz2, 20.f, 20000.f)
            || ! factoryInRange (e.db2, -12.f, 12.f) || ! factoryInRange (e.q2, 0.1f, 10.f))
            return false;
    }
    return true;
}

static_assert (factoryPresetCount >= 60, "factory preset bank");
static_assert (factoryBankOk(), "factory preset bank");

} // namespace k1x
