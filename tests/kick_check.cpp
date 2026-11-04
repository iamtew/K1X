#include "dsp/Distortion.h"
#include "dsp/EqBiquad.h"
#include "dsp/KickRender.h"
#include "dsp/Layers.h"
#include "presets/FactoryPresets.h"
#include "ui/VersionCompare.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

int main()
{
    k1x::KickVoice::Params p;
    std::vector<float> buf (44100);
    k1x::renderKick (buf.data(), (int) buf.size(), 44100.0, p);

    float peak = 0.f, energy = 0.f;
    for (float s : buf)
    {
        peak = std::max (peak, std::abs (s));
        energy += s * s;
    }
    assert (peak > 0.05f);
    assert (peak < 2.f);
    assert (energy > 1.f);

    p.lengthSec = 0.125f; // 1/16 at 120 BPM
    k1x::renderKick (buf.data(), (int) buf.size(), 44100.0, p);
    float tail = 0.f;
    const int after = (int) (0.125f * 44100.f) + 64;
    for (int i = after; i < (int) buf.size(); ++i)
        tail = std::max (tail, std::abs (buf[(size_t) i]));
    assert (tail < 1e-4f);

    const int gateFrom = (int) (0.015f * 44100.f);
    const int gateTo = (int) (0.125f * 44100.f);
    float maxDelta = 0.f;
    for (int i = gateFrom + 1; i < gateTo && i < (int) buf.size(); ++i)
        maxDelta = std::max (maxDelta, std::abs (buf[(size_t) i] - buf[(size_t) i - 1]));
    assert (maxDelta < 0.05f);

    const float q = k1x::lengthSeconds (1, 120.f);
    assert (std::abs (q - 0.5f) < 1e-4f);
    assert (k1x::lengthSeconds (0, 120.f) == 0.f);
    assert (std::abs (k1x::beatsToSeconds (0.25f, 120.f) - 0.125f) < 1e-4f); // 1/16

    assert (std::abs (k1x::midiNoteToHz (69) - 440.f) < 1e-3f);
    assert (std::abs (k1x::midiNoteToHz (57) - 220.f) < 1e-3f); // A3
    assert (std::abs (k1x::midiNoteToHz (81) - 880.f) < 1e-3f); // A5

    p.lengthSec = 0.f;
    std::vector<float> longBuf (88200);
    p.style = 1; // 808
    k1x::renderKick (longBuf.data(), (int) longBuf.size(), 44100.0, p);
    float boom = 0.f;
    const int late = (int) (1.3f * 44100.f);
    for (int i = late; i < (int) longBuf.size(); ++i)
        boom = std::max (boom, std::abs (longBuf[(size_t) i]));
    assert (boom > 0.01f);

    p.style = 3; // Tight
    k1x::renderKick (longBuf.data(), (int) longBuf.size(), 44100.0, p);
    float tightTail = 0.f;
    for (int i = late; i < (int) longBuf.size(); ++i)
        tightTail = std::max (tightTail, std::abs (longBuf[(size_t) i]));
    assert (tightTail < 1e-4f);
    assert (boom > tightTail);

    // Dry=1 FX=0 is a no-op mix; 0 dB peak EQ is unity at its centre.
    {
        float dry = 0.37f, wet = 0.91f;
        const float mixed = dry * 1.f + wet * 0.f;
        assert (std::abs (mixed - dry) < 1e-6f);
        auto b0 = k1x::eqBiquad (k1x::eqTypePeak, 44100.f, 1000.f, 0.7f, 0.f);
        const float mag0 = k1x::eqMagnitude (&b0, 1, 44100.f, 1000.f);
        assert (std::abs (mag0 - 1.f) < 1e-3f);
        auto b6 = k1x::eqBiquad (k1x::eqTypePeak, 44100.f, 1000.f, 0.7f, 6.f);
        const float mag6 = k1x::eqMagnitude (&b6, 1, 44100.f, 1000.f);
        const float db6 = 20.f * std::log10 (mag6);
        assert (std::abs (db6 - 6.f) < 0.25f);
    }

    assert (std::abs (k1x::punchToAttackMs (0.f) - 0.3f) < 1e-4f);
    assert (std::abs (k1x::punchToAttackMs (1.f) - 50.f) < 1e-4f);

    {
        k1x::KickVoice::Params h0;
        h0.drive = 0.f;
        h0.click = 0.f;
        h0.punch = 0.f;
        h0.pitchHold = 0.f;
        h0.ampHold = 0.f;
        std::vector<float> a (2048), b (2048);
        k1x::renderKick (a.data(), (int) a.size(), 44100.0, h0);
        h0.pitchHold = 0.05f;
        k1x::renderKick (b.data(), (int) b.size(), 44100.0, h0);
        float d = 0.f;
        for (int i = 0; i < (int) a.size(); ++i)
            d = std::max (d, std::abs (a[(size_t) i] - b[(size_t) i]));
        assert (d > 0.01f);

        h0.pitchHold = 0.f;
        h0.ampHold = 0.08f;
        k1x::renderKick (b.data(), (int) b.size(), 44100.0, h0);
        d = 0.f;
        for (int i = 0; i < (int) a.size(); ++i)
            d = std::max (d, std::abs (a[(size_t) i] - b[(size_t) i]));
        assert (d > 0.01f);
    }

    {
        k1x::KickVoice::Params a;
        a.punch = 0.f;
        std::vector<float> fast (44100), slow (44100);
        k1x::renderKick (fast.data(), (int) fast.size(), 44100.0, a);
        a.punch = 1.f;
        k1x::renderKick (slow.data(), (int) slow.size(), 44100.0, a);
        const int n10 = (int) (0.01f * 44100.f);
        float e0 = 0.f, e1 = 0.f;
        for (int i = 0; i < n10; ++i)
        {
            e0 += fast[(size_t) i] * fast[(size_t) i];
            e1 += slow[(size_t) i] * slow[(size_t) i];
        }
        assert (e1 < e0);
    }

    {
        const float drive = 5.f;
        const float x = 0.4f;
        assert (std::abs (k1x::distortSample (k1x::distSoft, x, drive, 0.5f) - std::tanh (x * drive)) < 1e-6f);
        const float hard = k1x::distortSample (k1x::distHard, 2.f, drive, 0.5f);
        assert (hard <= 1.f && hard >= -1.f);
        assert (std::abs (k1x::distortSample (k1x::distFold, x, drive, 0.5f)
                          - k1x::distortSample (k1x::distSoft, x, drive, 0.5f)) > 0.05f);
        float dc = 0.f;
        for (int i = 0; i < 8; ++i)
            dc += k1x::distortSample (k1x::distSoft, 0.f, drive, 0.2f);
        assert (std::abs (dc) < 1e-5f);
    }

    {
        k1x::KickVoice::Params late = p;
        late.delayMs = 20.f;
        late.lengthSec = 0.f;
        late.style = 0;
        k1x::renderKick (buf.data(), (int) buf.size(), 44100.0, late);
        float pre = 0.f;
        const int preN = (int) (0.015f * 44100.f);
        for (int i = 0; i < preN; ++i)
            pre = std::max (pre, std::abs (buf[(size_t) i]));
        assert (pre < 1.0e-6f);

        k1x::BassVoice::Params bass;
        bass.tuneHz = 40.f;
        bass.decay = 0.4f;
        bass.delayMs = 10.f;
        k1x::renderBass (buf.data(), (int) buf.size(), 44100.0, bass);
        pre = 0.f;
        const int bassPre = (int) (0.008f * 44100.f);
        for (int i = 0; i < bassPre; ++i)
            pre = std::max (pre, std::abs (buf[(size_t) i]));
        assert (pre < 1.0e-6f);
        float body = 0.f;
        for (int i = (int) (0.02f * 44100.f); i < (int) (0.08f * 44100.f); ++i)
            body = std::max (body, std::abs (buf[(size_t) i]));
        assert (body > 0.05f);

        k1x::NoiseVoice::Params nz;
        k1x::renderNoise (buf.data(), (int) buf.size(), 44100.0, nz);
        float nrg = 0.f;
        for (int i = 0; i < 4000; ++i)
            nrg += buf[(size_t) i] * buf[(size_t) i];
        assert (nrg > 0.01f);

        nz = {};
        nz.delayMs = 20.f;
        nz.lengthSec = 0.1f;
        k1x::renderNoise (buf.data(), (int) buf.size(), 44100.0, nz);
        pre = 0.f;
        const int nzPre = (int) (0.015f * 44100.f);
        for (int i = 0; i < nzPre; ++i)
            pre = std::max (pre, std::abs (buf[(size_t) i]));
        assert (pre < 1.0e-6f);
        nrg = 0.f;
        for (int i = (int) (0.025f * 44100.f); i < (int) (0.06f * 44100.f); ++i)
            nrg += buf[(size_t) i] * buf[(size_t) i];
        assert (nrg > 0.01f);

        {
            float impulse[64];
            for (int i = 0; i < 64; ++i)
                impulse[i] = i == 0 ? 1.f : 0.f;
            k1x::SampleVoice::Params sm;
            sm.data = impulse;
            sm.n = 64;
            sm.fileSr = 44100.0;
            sm.delayMs = 20.f;
            sm.hpfHz = 20.f;
            sm.lpfHz = 16000.f;
            k1x::renderSample (buf.data(), (int) buf.size(), 44100.0, sm);
            pre = 0.f;
            const int smPre = (int) (0.015f * 44100.f);
            for (int i = 0; i < smPre; ++i)
                pre = std::max (pre, std::abs (buf[(size_t) i]));
            assert (pre < 1.0e-6f);
            float smPeak = 0.f;
            for (int i = (int) (0.02f * 44100.f); i < (int) (0.025f * 44100.f); ++i)
                smPeak = std::max (smPeak, std::abs (buf[(size_t) i]));
            assert (smPeak > 0.01f);
        }

        nz = {};
        nz.decay = 1.f;
        nz.lengthSec = 0.02f;
        k1x::renderNoise (buf.data(), (int) buf.size(), 44100.0, nz);
        float nzTail = 0.f;
        for (int i = (int) (0.04f * 44100.f); i < (int) buf.size(); ++i)
            nzTail = std::max (nzTail, std::abs (buf[(size_t) i]));
        assert (nzTail < 1.0e-4f);

        auto bandRatio = [&] (k1x::NoiseVoice::Params p)
        {
            p.hpfHz = 80.f;
            p.lpfHz = 16000.f;
            p.decay = 0.4f;
            p.lengthSec = 0.2f;
            k1x::renderNoise (buf.data(), (int) buf.size(), 44100.0, p);
            float low = 0.f, high = 0.f, lp = 0.f, hp = 0.f;
            const float cLow = 1.f - std::exp (-6.283185307179586f * 400.f / 44100.f);
            const float cHigh = 1.f - std::exp (-6.283185307179586f * 4000.f / 44100.f);
            const int n = (int) (0.2f * 44100.f);
            for (int i = 0; i < n; ++i)
            {
                const float x = buf[(size_t) i];
                lp += cLow * (x - lp);
                low += lp * lp;
                hp += cHigh * (x - hp);
                const float hi = x - hp;
                high += hi * hi;
            }
            return low / (high + 1.0e-12f);
        };
        k1x::NoiseVoice::Params whiteP, pinkP;
        whiteP.type = k1x::NoiseVoice::white;
        pinkP.type = k1x::NoiseVoice::pink;
        assert (bandRatio (pinkP) > bandRatio (whiteP) * 1.2f);

        k1x::ClickVoice::Params ck;
        ck.decayMs = 5.f;
        ck.lengthSec = 0.f;
        k1x::renderClickLayer (buf.data(), (int) buf.size(), 44100.0, ck);
        float clickTail = 0.f;
        for (int i = (int) (0.05f * 44100.f); i < (int) buf.size(); ++i)
            clickTail = std::max (clickTail, std::abs (buf[(size_t) i]));
        assert (clickTail < 1.0e-4f);

        ck = {};
        ck.decayMs = 40.f;
        ck.lengthSec = 0.01f;
        k1x::renderClickLayer (buf.data(), (int) buf.size(), 44100.0, ck);
        clickTail = 0.f;
        for (int i = (int) (0.03f * 44100.f); i < (int) buf.size(); ++i)
            clickTail = std::max (clickTail, std::abs (buf[(size_t) i]));
        assert (clickTail < 1.0e-4f);

        auto clickLow = [&] (float hpHz)
        {
            k1x::ClickVoice::Params p;
            p.tone = 0.f;
            p.decayMs = 30.f;
            p.lengthSec = 0.08f;
            p.hpfHz = hpHz;
            p.lpfHz = 16000.f;
            k1x::renderClickLayer (buf.data(), (int) buf.size(), 44100.0, p);
            float lp = 0.f, e = 0.f;
            const float c = 1.f - std::exp (-6.283185307179586f * 400.f / 44100.f);
            const int n = (int) (0.08f * 44100.f);
            for (int i = 0; i < n; ++i)
            {
                const float x = buf[(size_t) i];
                lp += c * (x - lp);
                e += lp * lp;
            }
            return e;
        };
        assert (clickLow (80.f) > clickLow (3000.f) * 1.2f);

        k1x::SweepVoice::Params sw;
        k1x::renderSweep (buf.data(), (int) buf.size(), 44100.0, sw);
        float swPeak = 0.f;
        for (int i = 0; i < 8000; ++i)
            swPeak = std::max (swPeak, std::abs (buf[(size_t) i]));
        assert (swPeak > 0.05f);

        auto sweepHigh = [&] (float drive)
        {
            k1x::SweepVoice::Params p;
            p.startHz = 400.f;
            p.dropOct = 0.f;
            p.decay = 0.2f;
            p.lengthSec = 0.15f;
            p.drive = drive;
            p.lpfHz = 8000.f;
            k1x::renderSweep (buf.data(), (int) buf.size(), 44100.0, p);
            float hp = 0.f, e = 0.f;
            const float c = 1.f - std::exp (-6.283185307179586f * 800.f / 44100.f);
            const int n = (int) (0.15f * 44100.f);
            for (int i = 0; i < n; ++i)
            {
                const float x = buf[(size_t) i];
                hp += c * (x - hp);
                e += (x - hp) * (x - hp);
            }
            return e;
        };
        assert (sweepHigh (1.f) > sweepHigh (0.f) * 1.5f);

        sw = {};
        sw.decay = 0.5f;
        sw.lengthSec = 0.02f;
        k1x::renderSweep (buf.data(), (int) buf.size(), 44100.0, sw);
        float swTail = 0.f;
        for (int i = (int) (0.05f * 44100.f); i < (int) buf.size(); ++i)
            swTail = std::max (swTail, std::abs (buf[(size_t) i]));
        assert (swTail < 1.0e-4f);

        bass = {};
        bass.tuneHz = 50.f;
        bass.decay = 1.f;
        bass.lengthSec = 0.03f;
        k1x::renderBass (buf.data(), (int) buf.size(), 44100.0, bass);
        float bassTail = 0.f;
        for (int i = (int) (0.06f * 44100.f); i < (int) buf.size(); ++i)
            bassTail = std::max (bassTail, std::abs (buf[(size_t) i]));
        assert (bassTail < 1.0e-4f);

        bass = {};
        bass.tuneHz = 50.f;
        bass.decay = 0.4f;
        bass.attackMs = 25.f;
        bass.lengthSec = 0.2f;
        k1x::renderBass (buf.data(), (int) buf.size(), 44100.0, bass);
        assert (std::abs (buf[0]) < 1.0e-4f);
        float atkLater = 0.f;
        for (int i = (int) (0.02f * 44100.f); i < (int) (0.03f * 44100.f); ++i)
            atkLater = std::max (atkLater, std::abs (buf[(size_t) i]));
        assert (atkLater > 0.02f);

        auto bassZc = [&] (float drop)
        {
            k1x::BassVoice::Params p;
            p.tuneHz = 50.f;
            p.dropOct = drop;
            p.decay = 0.4f;
            p.lengthSec = 0.05f;
            k1x::renderBass (buf.data(), (int) buf.size(), 44100.0, p);
            int zc = 0;
            const int n = (int) (0.02f * 44100.f);
            for (int i = 1; i < n; ++i)
                if ((buf[(size_t) i - 1] < 0.f) != (buf[(size_t) i] < 0.f))
                    ++zc;
            return zc;
        };
        assert (bassZc (2.f) > bassZc (0.f) * 1.3f);

        float gain[4] = { 1.f, 0.5f, 1.f, 1.f };
        bool mute[4] = { false, false, true, false };
        bool solo[4] = { false, true, false, false };
        bool enabled[4] = { true, true, true, false };
        bool flip[4] = { true, false, false, false };
        float amp[4];
        k1x::layerAmps (gain, mute, solo, enabled, flip, amp);
        assert (amp[0] == 0.f);
        assert (std::abs (amp[1] - 0.5f) < 1.0e-6f);
        assert (amp[2] == 0.f);
        assert (amp[3] == 0.f);

        float layer[4] = { 1.f, 1.f, 1.f, 1.f };
        float sendAmt[4] = { 1.f, 0.f, 1.f, 1.f };
        float unit[4] = { 1.f, 1.f, 1.f, 1.f };
        float dry = 0.f, send = 0.f;
        k1x::layerSendMix (layer, unit, sendAmt, dry, send);
        assert (std::abs (dry - 4.f) < 1.0e-6f);
        assert (std::abs (send - 3.f) < 1.0e-6f);
        float allSend[4] = { 1.f, 1.f, 1.f, 1.f };
        k1x::layerSendMix (layer, unit, allSend, dry, send);
        assert (std::abs (dry - send) < 1.0e-6f);

        const float sub = 100.f * std::pow (2.f, -12.f / 12.f);
        assert (std::abs (sub - 50.f) < 1.0e-3f);

        k1x::LayerStack stack;
        stack.prepare (44100.0);
        k1x::KickVoice::Params kp;
        kp.tuneHz = 50.f;
        kp.decay = 0.3f;
        kp.level = 1.f;
        k1x::SlotVoiceParams slot[3];
        slot[0].type = k1x::layerBass;
        slot[0].bass.tuneHz = sub * 0.5f;
        slot[0].bass.decay = 0.3f;
        stack.trigger (kp, slot);
        float both = 0.f;
        for (int i = 0; i < 2000; ++i)
        {
            float layer[4];
            stack.tickInto (layer);
            both += (layer[0] + layer[1]) * (layer[0] + layer[1]);
        }
        assert (both > 0.05f);

        // Retrigger on a loud sample: splice step must be small vs the tail.
        {
            k1x::LayerStack st;
            st.prepare (44100.0);
            k1x::KickVoice::Params p;
            p.tuneHz = 50.f;
            p.decay = 0.5f;
            p.punch = 0.2f;
            p.click = 0.f;
            st.trigger (p, nullptr);
            bool hit = false;
            for (int i = 0; i < 8000; ++i)
            {
                float layer[4];
                st.tickInto (layer);
                if (i < 200 || std::abs (layer[0]) < 0.15f)
                    continue;
                const float tail = layer[0];
                st.trigger (p, nullptr);
                float yPrev = tail;
                float maxStep = 0.f;
                const int win = (int) (0.005 * 44100);
                for (int j = 0; j < win; ++j)
                {
                    st.tickInto (layer);
                    maxStep = std::max (maxStep, std::abs (layer[0] - yPrev));
                    yPrev = layer[0];
                }
                assert (maxStep < std::abs (tail) * 0.2f);
                hit = true;
                break;
            }
            assert (hit);
        }
    }

    assert (k1x::factoryPresetCount >= 60);
    assert (k1x::factoryBankOk());
    assert (k1x::factoryStackIndex ("Punch Stack") == 0);
    assert (k1x::factoryStackIndex ("Punch") < 0);
    assert (k1x::stackFor (0).kickDelayMs == 1.f);
    assert (k1x::stackFor (0).layer[0].type == 3);
    assert (k1x::stackFor (0).layer[0].delayMs == 0.f);
    assert (k1x::stackFor (0).layer[1].type == 1);
    assert (k1x::stackFor (0).layer[1].delayMs == 4.f);
    assert (k1x::stackFor (6).layer[1].type == 1);
    assert (k1x::stackFor (6).layer[1].delayMs == 4.f);
    assert (k1x::stackFor (0).layer[0].a != k1x::stackFor (6).layer[0].a);

    std::vector<std::string> tags { "0.0.2", "v0.0.1" };
    assert (k1x::newestNewerTag (tags, "0.0.1") == "0.0.2");
    assert (k1x::newestNewerTag (tags, "0.0.2").empty());

    std::printf ("kick_check ok  peak=%.3f energy=%.1f\n", peak, energy);
    return 0;
}
