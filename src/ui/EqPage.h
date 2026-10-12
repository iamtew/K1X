#pragma once

#include "../PluginProcessor.h"
#include "../dsp/EqBiquad.h"
#include "../dsp/ParametricEq.h"

#include <JuceHeader.h>

// Post EQ plot + inspector. Spectrum is the post-FX, post-EQ output.
class EqPage : public juce::Component,
               private juce::Timer
{
public:
    explicit EqPage (K1XAudioProcessor& p) : proc (p), curve (*this), inspector (*this)
    {
        eqOn.setButtonText ("EQ");
        addAndMakeVisible (eqOn);
        eqOnAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
            proc.apvts, ParamID::postEqOn, eqOn);

        addAndMakeVisible (curve);
        addAndMakeVisible (inspector);
        resetBtn.setButtonText ("Reset");
        resetBtn.setClickingTogglesState (false);
        resetBtn.onClick = [this] { resetEq(); };
        addAndMakeVisible (resetBtn);
        inspector.bind();
        startTimerHz (24);
    }

    ~EqPage() override { stopTimer(); }

    void resized() override
    {
        auto r = getLocalBounds();
        auto rail = r.removeFromLeft (118);
        auto top = rail.removeFromTop (22).reduced (4, 1);
        eqOn.setBounds (top.removeFromLeft (top.getWidth() / 2));
        resetBtn.setBounds (top);
        inspector.setBounds (rail);
        curve.setBounds (r);
    }

    K1XAudioProcessor& proc;

private:
    static const juce::Colour kMetal;
    static const juce::Colour kGraph;
    static const juce::Colour kSum;
    static const juce::Colour kBandCol[k1x::eqBandCount];

    static juce::Font labelFont (float h)
    {
        return juce::Font (juce::FontOptions (h).withStyle ("Bold"));
    }

    void timerCallback() override
    {
        inspector.syncPower();
        curve.repaint();
    }

    void selectBand (int b)
    {
        selBand = juce::jlimit (0, k1x::eqBandCount - 1, b);
        inspector.bind();
        repaint();
    }

    const char* typeId (int b) const { return ParamID::postEqType[b]; }
    const char* freqId (int b) const { return ParamID::postEqFreq[b]; }
    const char* gainId (int b) const { return ParamID::postEqGain[b]; }
    const char* qId (int b) const { return ParamID::postEqQ[b]; }

    void setParam (const char* id, float v)
    {
        auto* p = dynamic_cast<juce::RangedAudioParameter*> (proc.apvts.getParameter (id));
        if (p == nullptr)
            return;
        p->setValueNotifyingHost (p->convertTo0to1 (p->getNormalisableRange().snapToLegalValue (v)));
    }

    void beginParam (const char* id)
    {
        if (auto* p = proc.apvts.getParameter (id))
            p->beginChangeGesture();
    }
    void endParam (const char* id)
    {
        if (auto* p = proc.apvts.getParameter (id))
            p->endChangeGesture();
    }

    float loadId (const char* id) const
    {
        auto* v = proc.apvts.getRawParameterValue (id);
        return v != nullptr ? v->load() : 0.f;
    }

    void setType (int b, int type)
    {
        auto* p = proc.apvts.getParameter (typeId (b));
        if (p == nullptr)
            return;
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 ((float) type));
        p->endChangeGesture();
    }

    void resetParamId (const char* id)
    {
        auto* p = proc.apvts.getParameter (id);
        if (p == nullptr)
            return;
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->getDefaultValue());
        p->endChangeGesture();
    }

    void resetBand (int b)
    {
        resetParamId (typeId (b));
        resetParamId (freqId (b));
        resetParamId (gainId (b));
        resetParamId (qId (b));
        inspector.bind();
        curve.repaint();
    }

    void resetEq()
    {
        resetParamId (ParamID::postEqOn);
        for (int b = 0; b < k1x::eqBandCount; ++b)
        {
            resetParamId (typeId (b));
            resetParamId (freqId (b));
            resetParamId (gainId (b));
            resetParamId (qId (b));
        }
        inspector.bind();
        curve.repaint();
    }

    struct BandKnob : public juce::Component
    {
        BandKnob()
        {
            label.setJustificationType (juce::Justification::centred);
            label.setFont (labelFont (11.f));
            label.setColour (juce::Label::textColourId, kMetal);
            addAndMakeVisible (label);
            slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 14);
            slider.setColour (juce::Slider::textBoxTextColourId, kMetal);
            slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
            slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            addAndMakeVisible (slider);
        }

        void bind (K1XAudioProcessor& p, const char* id, const juce::String& name)
        {
            label.setText (name, juce::dontSendNotification);
            att.reset();
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p.apvts.getParameter (id)))
                slider.setDoubleClickReturnValue (true, ranged->convertFrom0to1 (ranged->getDefaultValue()));
            att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                p.apvts, id, slider);
        }

        void resized() override
        {
            auto r = getLocalBounds();
            label.setBounds (r.removeFromTop (14));
            slider.setBounds (r);
        }

        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };

    struct Inspector : public juce::Component
    {
        explicit Inspector (EqPage& page) : owner (page)
        {
            power.setButtonText ("On");
            power.onClick = [this]
            {
                const int t = juce::roundToInt (owner.loadId (owner.typeId (owner.selBand)));
                owner.setType (owner.selBand, t > k1x::eqTypeOff ? k1x::eqTypeOff : k1x::eqTypePeak);
                bind();
            };
            prev.setButtonText ("<");
            next.setButtonText (">");
            prev.setClickingTogglesState (false);
            next.setClickingTogglesState (false);
            prev.onClick = [this] { owner.selectBand (owner.selBand - 1); };
            next.onClick = [this] { owner.selectBand (owner.selBand + 1); };
            combo.setColour (juce::ComboBox::textColourId, kMetal);
            combo.setColour (juce::ComboBox::arrowColourId, kMetal);
            addAndMakeVisible (power);
            addAndMakeVisible (prev);
            addAndMakeVisible (next);
            addAndMakeVisible (combo);
            addAndMakeVisible (freq);
            addAndMakeVisible (gain);
            addAndMakeVisible (q);
        }

        void bind()
        {
            const int b = owner.selBand;
            comboAtt.reset();
            combo.clear (juce::dontSendNotification);
            auto* choice = dynamic_cast<juce::AudioParameterChoice*> (
                owner.proc.apvts.getParameter (owner.typeId (b)));
            if (choice != nullptr)
            {
                for (int i = 0; i < choice->choices.size(); ++i)
                    combo.addItem (choice->choices[i], i + 1);
                comboAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
                    owner.proc.apvts, owner.typeId (b), combo);
            }
            freq.bind (owner.proc, owner.freqId (b), "FREQ");
            gain.bind (owner.proc, owner.gainId (b), "GAIN");
            q.bind (owner.proc, owner.qId (b), "Q");
            syncPower();
            repaint();
        }

        void syncPower()
        {
            const int t = juce::roundToInt (owner.loadId (owner.typeId (owner.selBand)));
            power.setToggleState (t > k1x::eqTypeOff, juce::dontSendNotification);
        }

        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            g.setColour (juce::Colour (0xee151b24));
            g.fillRoundedRectangle (r, 6.f);
            g.setColour (kBandCol[owner.selBand]);
            g.drawRoundedRectangle (r.reduced (0.8f), 6.f, 1.6f);
            g.setColour (kMetal);
            g.setFont (labelFont (16.f));
            auto mid = juce::Rectangle<int> (getWidth() / 2 - 14, 8, 28, 22);
            g.drawText (juce::String (owner.selBand + 1), mid, juce::Justification::centred, false);
        }

        void resized() override
        {
            auto r = getLocalBounds().reduced (6, 6);
            auto row = r.removeFromTop (24);
            power.setBounds (row.removeFromLeft (36));
            prev.setBounds (row.removeFromLeft (22).reduced (1));
            next.setBounds (row.removeFromRight (22).reduced (1));
            combo.setBounds (r.removeFromTop (24).reduced (0, 1));
            const int h = juce::jmax (1, r.getHeight() / 3);
            freq.setBounds (r.removeFromTop (h));
            gain.setBounds (r.removeFromTop (h));
            q.setBounds (r);
        }

        EqPage& owner;
        juce::ToggleButton power;
        juce::ToggleButton prev, next;
        juce::ComboBox combo;
        BandKnob freq, gain, q;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAtt;
    };

    struct Curve : public juce::Component
    {
        explicit Curve (EqPage& page) : owner (page) { setMouseCursor (juce::MouseCursor::CrosshairCursor); }

        float sr() const
        {
            const double s = owner.proc.getSampleRate();
            return s > 1.0 ? (float) s : 44100.f;
        }

        juce::Rectangle<float> plot() const
        {
            return getLocalBounds().toFloat().reduced (28.f, 18.f);
        }

        static float xToHz (float x01)
        {
            return 20.f * std::pow (10.f, juce::jlimit (0.f, 1.f, x01) * 3.f);
        }
        static float hzToX (float hz)
        {
            hz = juce::jlimit (20.f, 20000.f, hz);
            return std::log10 (hz / 20.f) / 3.f;
        }
        static float yToDb (float y01)
        {
            return (1.f - juce::jlimit (0.f, 1.f, y01)) * 36.f - 18.f;
        }
        static float dbToY (float db)
        {
            return 1.f - juce::jlimit (0.f, 1.f, (db + 18.f) / 36.f);
        }
        static float specToY (float db)
        {
            return 1.f - juce::jlimit (0.f, 1.f, (db + 72.f) / 72.f);
        }

        juce::Point<float> bandPos (const k1x::EqBandParam& p, const k1x::Biquad& bq) const
        {
            auto r = plot();
            float db = p.gainDb;
            if (p.type == k1x::eqTypeHP || p.type == k1x::eqTypeLP)
            {
                k1x::Biquad one[1] { bq };
                const float mag = k1x::eqMagnitude (one, 1, sr(), p.hz);
                db = mag > 1.0e-8f ? 20.f * std::log10 (mag) : -18.f;
            }
            return { r.getX() + hzToX (p.hz) * r.getWidth(),
                     r.getY() + dbToY (db) * r.getHeight() };
        }

        int hitBand (juce::Point<float> pos, const k1x::EqBandParam* params, const k1x::Biquad* bq) const
        {
            int best = -1;
            float bestD = 16.f * 16.f;
            for (int b = 0; b < k1x::eqBandCount; ++b)
            {
                if (params[b].type <= k1x::eqTypeOff)
                    continue;
                const auto pt = bandPos (params[b], bq[b]);
                const float d = pos.getDistanceSquaredFrom (pt);
                if (d < bestD)
                {
                    bestD = d;
                    best = b;
                }
            }
            return best;
        }

        void paint (juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            g.setColour (kGraph);
            g.fillRoundedRectangle (bounds, 4.f);

            auto r = plot();
            g.setColour (juce::Colour (0xff2a3444));
            const float zeroY = r.getY() + dbToY (0.f) * r.getHeight();
            g.drawHorizontalLine ((int) zeroY, r.getX(), r.getRight());
            for (int db : { 12, 6, -6, -12 })
                g.drawHorizontalLine ((int) (r.getY() + dbToY ((float) db) * r.getHeight()),
                                      r.getX(), r.getRight());
            const float gridHz[] = { 20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f };
            for (float hz : gridHz)
            {
                const float x = r.getX() + hzToX (hz) * r.getWidth();
                g.drawVerticalLine ((int) x, r.getY(), r.getBottom());
            }

            const int n = juce::jmax (2, (int) r.getWidth());
            if ((int) specDb.size() != n)
                specDb.resize ((size_t) n);
            owner.proc.eqAnalyser().copyLogDb (specDb.data(), n);
            juce::Path spec;
            spec.startNewSubPath (r.getX(), r.getBottom());
            for (int i = 0; i < n; ++i)
            {
                const float x01 = (float) i / (float) (n - 1);
                spec.lineTo (r.getX() + x01 * r.getWidth(),
                             r.getY() + specToY (specDb[(size_t) i]) * r.getHeight());
            }
            spec.lineTo (r.getRight(), r.getBottom());
            spec.closeSubPath();
            g.setColour (juce::Colour (0x55d94214));
            g.fillPath (spec);

            k1x::EqBandParam params[k1x::eqBandCount];
            k1x::readEqBands (owner.proc.apvts, params);
            k1x::Biquad bq[k1x::eqBandCount];
            for (int b = 0; b < k1x::eqBandCount; ++b)
                bq[b] = k1x::eqBiquad (params[b].type, sr(), params[b].hz, params[b].q, params[b].gainDb);

            auto strokeBand = [&] (int b, float alpha, float thick)
            {
                if (params[b].type <= k1x::eqTypeOff)
                    return;
                k1x::Biquad one[1] { bq[b] };
                juce::Path path;
                for (int i = 0; i < n; ++i)
                {
                    const float x01 = (float) i / (float) (n - 1);
                    const float mag = k1x::eqMagnitude (one, 1, sr(), xToHz (x01));
                    const float db = mag > 1.0e-8f ? 20.f * std::log10 (mag) : -80.f;
                    const juce::Point<float> pt (r.getX() + x01 * r.getWidth(),
                                                 r.getY() + dbToY (db) * r.getHeight());
                    if (i == 0)
                        path.startNewSubPath (pt);
                    else
                        path.lineTo (pt);
                }
                g.setColour (kBandCol[b].withAlpha (alpha));
                g.strokePath (path, juce::PathStrokeType (thick));
            };
            for (int b = 0; b < k1x::eqBandCount; ++b)
                strokeBand (b, b == owner.selBand ? 0.85f : 0.4f, b == owner.selBand ? 1.8f : 1.2f);

            juce::Path sum;
            juce::Path fill;
            const bool on = k1x::postEqEnabled (owner.proc.apvts);
            for (int i = 0; i < n; ++i)
            {
                const float x01 = (float) i / (float) (n - 1);
                const float mag = k1x::eqMagnitude (bq, k1x::eqBandCount, sr(), xToHz (x01));
                const float db = mag > 1.0e-8f ? 20.f * std::log10 (mag) : -80.f;
                const juce::Point<float> pt (r.getX() + x01 * r.getWidth(),
                                             r.getY() + dbToY (db) * r.getHeight());
                if (i == 0)
                {
                    sum.startNewSubPath (pt);
                    fill.startNewSubPath (pt.x, zeroY);
                    fill.lineTo (pt);
                }
                else
                {
                    sum.lineTo (pt);
                    fill.lineTo (pt);
                }
            }
            fill.lineTo (r.getRight(), zeroY);
            fill.closeSubPath();
            g.setColour (kSum.withAlpha (on ? 0.16f : 0.06f));
            g.fillPath (fill);
            g.setColour (kSum.withAlpha (on ? 1.f : 0.35f));
            g.strokePath (sum, juce::PathStrokeType (2.f));

            if (hoverX >= 0.f)
            {
                g.setColour (juce::Colour (0x66b7c5d6));
                g.drawLine (hoverX, r.getY(), hoverX, r.getBottom(), 1.f);
            }

            g.setFont (labelFont (10.f));
            for (int b = 0; b < k1x::eqBandCount; ++b)
            {
                if (params[b].type <= k1x::eqTypeOff)
                    continue;
                const auto pt = bandPos (params[b], bq[b]);
                const bool sel = owner.selBand == b;
                g.setColour (kBandCol[b]);
                g.fillEllipse (pt.x - (sel ? 6.f : 5.f), pt.y - (sel ? 6.f : 5.f),
                               sel ? 12.f : 10.f, sel ? 12.f : 10.f);
                if (sel)
                {
                    g.setColour (kMetal);
                    g.drawEllipse (pt.x - 7.f, pt.y - 7.f, 14.f, 14.f, 1.4f);
                }
            }

            g.setColour (juce::Colour (0xff8a9098));
            g.setFont (labelFont (10.f));
            auto freqLabel = [&] (float hz, const char* text)
            {
                g.drawText (text, (int) (r.getX() + hzToX (hz) * r.getWidth()) - 14,
                            (int) r.getBottom() + 2, 28, 12, juce::Justification::centred, false);
            };
            freqLabel (20.f, "20");
            freqLabel (100.f, "100");
            freqLabel (1000.f, "1k");
            freqLabel (10000.f, "10k");
            freqLabel (20000.f, "20k");
            g.drawText ("+12", (int) r.getRight() + 2, (int) (r.getY() + dbToY (12.f) * r.getHeight()) - 6,
                        24, 12, juce::Justification::centredLeft, false);
            g.drawText ("0", (int) r.getRight() + 2, (int) zeroY - 6,
                        24, 12, juce::Justification::centredLeft, false);
            g.drawText ("-12", (int) r.getRight() + 2, (int) (r.getY() + dbToY (-12.f) * r.getHeight()) - 6,
                        24, 12, juce::Justification::centredLeft, false);
        }

        void mouseMove (const juce::MouseEvent& e) override
        {
            hoverX = e.position.x;
            repaint();
        }
        void mouseExit (const juce::MouseEvent&) override
        {
            hoverX = -1.f;
            repaint();
        }

        void mouseDown (const juce::MouseEvent& e) override
        {
            k1x::EqBandParam params[k1x::eqBandCount];
            k1x::readEqBands (owner.proc.apvts, params);
            k1x::Biquad bq[k1x::eqBandCount];
            for (int b = 0; b < k1x::eqBandCount; ++b)
                bq[b] = k1x::eqBiquad (params[b].type, sr(), params[b].hz, params[b].q, params[b].gainDb);
            drag = hitBand (e.position, params, bq);
            if (drag < 0)
                return;
            owner.selectBand (drag);
            if (e.getNumberOfClicks() >= 2)
            {
                owner.resetBand (drag);
                drag = -1;
                return;
            }
            owner.beginParam (owner.freqId (drag));
            owner.beginParam (owner.gainId (drag));
            gesturing = true;
        }

        void mouseDrag (const juce::MouseEvent& e) override
        {
            hoverX = e.position.x;
            if (drag < 0)
                return;
            auto r = plot();
            const float x01 = r.getWidth() > 1.f ? (e.position.x - r.getX()) / r.getWidth() : 0.f;
            const float y01 = r.getHeight() > 1.f ? (e.position.y - r.getY()) / r.getHeight() : 0.f;
            owner.setParam (owner.freqId (drag), xToHz (x01));
            owner.setParam (owner.gainId (drag), yToDb (y01));
            repaint();
        }

        void mouseUp (const juce::MouseEvent&) override
        {
            if (! gesturing)
                return;
            owner.endParam (owner.freqId (drag));
            owner.endParam (owner.gainId (drag));
            gesturing = false;
            drag = -1;
            owner.inspector.bind();
        }

        void mouseDoubleClick (const juce::MouseEvent& e) override
        {
            k1x::EqBandParam params[k1x::eqBandCount];
            k1x::readEqBands (owner.proc.apvts, params);
            k1x::Biquad bq[k1x::eqBandCount];
            for (int b = 0; b < k1x::eqBandCount; ++b)
                bq[b] = k1x::eqBiquad (params[b].type, sr(), params[b].hz, params[b].q, params[b].gainDb);
            const int b = hitBand (e.position, params, bq);
            if (b < 0)
                return;
            owner.resetBand (b);
        }

        void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
        {
            k1x::EqBandParam params[k1x::eqBandCount];
            k1x::readEqBands (owner.proc.apvts, params);
            k1x::Biquad bq[k1x::eqBandCount];
            for (int b = 0; b < k1x::eqBandCount; ++b)
                bq[b] = k1x::eqBiquad (params[b].type, sr(), params[b].hz, params[b].q, params[b].gainDb);
            int b = hitBand (e.position, params, bq);
            if (b < 0)
                b = owner.selBand;
            const char* id = owner.qId (b);
            owner.beginParam (id);
            owner.setParam (id, owner.loadId (id) * (1.f + w.deltaY * 0.25f) + w.deltaY * 0.05f);
            owner.endParam (id);
            owner.selectBand (b);
        }

        EqPage& owner;
        int drag = -1;
        bool gesturing = false;
        float hoverX = -1.f;
        std::vector<float> specDb;
    };

    juce::ToggleButton eqOn;
    juce::ToggleButton resetBtn;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> eqOnAtt;
    Curve curve;
    Inspector inspector;
    int selBand = 0;
};

inline const juce::Colour EqPage::kMetal { 0xffb7c5d6 };
inline const juce::Colour EqPage::kGraph { 0xff12151a };
inline const juce::Colour EqPage::kSum { 0xffd94214 };
inline const juce::Colour EqPage::kBandCol[k1x::eqBandCount] {
    juce::Colour (0xffe07a2e),
    juce::Colour (0xff4aa3ff),
    juce::Colour (0xffc86ad8),
    juce::Colour (0xff3dcc8c),
    juce::Colour (0xffe8c200)
};
