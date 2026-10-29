#include "PluginEditor.h"
#include "presets/FactoryPresets.h"
#include "ui/VersionCompare.h"

#include <cmath>
#include <thread>
#include <vector>

namespace
{
constexpr int kEditorW = 720;
constexpr int kEditorH = 460;
constexpr const char* kPageNames[] = { "Main", "Effects" };
constexpr int kPageCount = 2;
constexpr int kPad = 8;
constexpr int kTitleH = 40;
constexpr int kTitleHitW = 150;
constexpr int kLabelH = 12;

const juce::Colour kChassis { 0xff1a2332 };
const juce::Colour kMetal   { 0xffb7c5d6 };
const juce::Colour kDark    { 0xff151515 };
const juce::Colour kOrange  { 0xffd94214 };
const juce::Colour kWhite   { 0xffffffff };

juce::Font boldFont (float h)
{
    return juce::Font (juce::FontOptions (h).withStyle ("Bold"));
}

constexpr int kMenuInit = 1;
constexpr int kMenuSave = 2;
constexpr int kMenuSaveAs = 3;
constexpr int kMenuExport = 4;
constexpr int kMenuImport = 5;
constexpr int kMenuFactory = 1000;
constexpr int kMenuUser = 10000;
} // namespace

K1XAudioProcessorEditor::Knob::Knob (juce::AudioProcessorValueTreeState& st,
                                     const juce::String& id, const juce::String& name)
{
    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, kMetal);
    label.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (label);

    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 12);
    slider.setColour (juce::Slider::textBoxTextColourId, kMetal);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);
    att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (st, id, slider);
}

void K1XAudioProcessorEditor::Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (kLabelH));
    slider.setBounds (r);
}

K1XAudioProcessorEditor::MixStrip::MixStrip (juce::AudioProcessorValueTreeState& st, const char* id,
                                             const char* name, k1x::OutputMeter& m)
    : meter (m)
{
    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setColour (juce::Label::textColourId, kMetal);
    label.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (label);

    slider.setLookAndFeel (&lnf);
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 40, 12);
    slider.setOpaque (false);
    slider.setColour (juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::trackColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::thumbColourId, kMetal);
    slider.setColour (juce::Slider::textBoxTextColourId, kMetal);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (st.getParameter (id)))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (st, id, slider);
    addAndMakeVisible (slider);
}

K1XAudioProcessorEditor::MixStrip::~MixStrip()
{
    slider.setLookAndFeel (nullptr);
}

juce::Rectangle<float> K1XAudioProcessorEditor::MixStrip::trackArea() const
{
    auto r = slider.getBounds().toFloat();
    r.removeFromBottom (12.f); // percent box
    return r.reduced (10.f, 4.f);
}

void K1XAudioProcessorEditor::MixStrip::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (kLabelH));
    slider.setBounds (r);
}

void K1XAudioProcessorEditor::MixStrip::LnF::drawLinearSlider (juce::Graphics& g, int x, int, int w, int,
                                                              float pos, float, float,
                                                              juce::Slider::SliderStyle, juce::Slider&)
{
    // Cap only — meter fill is MixStrip::paint.
    const float cx = (float) x + (float) w * 0.5f;
    juce::Rectangle<float> cap (cx - 12.f, pos - 4.f, 24.f, 8.f);
    g.setColour (kDark);
    g.fillRoundedRectangle (cap, 2.f);
    g.setColour (kMetal);
    g.drawRoundedRectangle (cap, 2.f, 1.f);
}

void K1XAudioProcessorEditor::MixStrip::paint (juce::Graphics& g)
{
    auto track = trackArea();
    g.setColour (kDark);
    g.fillRoundedRectangle (track, 3.f);

    const float peak = meter.lastPeak();
    const float db = peak > 1.0e-6f ? 20.f * std::log10 (peak) : -90.f;
    const float y01 = juce::jlimit (0.f, 1.f, (db + 48.f) / 48.f);
    if (y01 > 0.f)
    {
        auto fill = track.removeFromBottom (track.getHeight() * y01);
        g.setColour (clipped ? kOrange : kChassis.brighter (0.15f));
        g.fillRoundedRectangle (fill, 3.f);
    }
}

void K1XAudioProcessorEditor::MixStrip::refresh()
{
    if (meter.takeClip())
        clipUntil = juce::Time::getMillisecondCounter() + 800;
    clipped = juce::Time::getMillisecondCounter() < clipUntil;
    repaint();
}

K1XAudioProcessorEditor::FxPage::FxPage (K1XAudioProcessor& p)
    : proc (p),
      dryStrip (p.apvts, ParamID::fxDry, "Dry", p.dryMeter()),
      fxStrip (p.apvts, ParamID::fxWet, "FX", p.fxMeter()),
      eq (p)
{
    auto setupOn = [this, &p] (juce::ToggleButton& b, const char* id,
                               std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& att)
    {
        addAndMakeVisible (b);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, id, b);
    };
    setupOn (dstOn, ParamID::fxDstOn, dstOnAtt);
    setupOn (dlyOn, ParamID::fxDlyOn, dlyOnAtt);
    setupOn (dlySync, ParamID::fxDlySync, dlySyncAtt);
    setupOn (revOn, ParamID::fxRevOn, revOnAtt);
    setupOn (revSync, ParamID::fxRevSync, revSyncAtt);

    struct Spec { const char* id; const char* name; };
    static constexpr Spec kFxKnobs[] = {
        { ParamID::fxDstDrive, "Drive" },
        { ParamID::fxDlyTime, "Time" }, { ParamID::fxDlyFb, "Fb" }, { ParamID::fxDlyCut, "Cut" },
        { ParamID::fxRevPredelay, "Pre" }, { ParamID::fxRevSize, "Size" },
        { ParamID::fxRevDamp, "Damp" }, { ParamID::fxRevWidth, "Width" }
    };
    for (auto& s : kFxKnobs)
    {
        auto* k = knobs.add (new Knob (p.apvts, s.id, s.name));
        addAndMakeVisible (k);
    }
    addAndMakeVisible (dryStrip);
    addAndMakeVisible (fxStrip);

    dlyDivLabel.setText ("Div", juce::dontSendNotification);
    dlyDivLabel.setJustificationType (juce::Justification::centred);
    dlyDivLabel.setColour (juce::Label::textColourId, kMetal);
    addAndMakeVisible (dlyDivLabel);
    dlyDiv.addItem ("1/16", 1);
    dlyDiv.addItem ("1/8", 2);
    dlyDiv.addItem ("1/4", 3);
    dlyDiv.addItem ("1/2", 4);
    dlyDiv.setColour (juce::ComboBox::textColourId, kMetal);
    dlyDiv.setColour (juce::ComboBox::arrowColourId, kMetal);
    addAndMakeVisible (dlyDiv);
    dlyDivAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, ParamID::fxDlyDiv, dlyDiv);

    p.apvts.addParameterListener (ParamID::fxRevSync, this);
    bindPreKnob();
    addAndMakeVisible (eq);
}

K1XAudioProcessorEditor::FxPage::~FxPage()
{
    proc.apvts.removeParameterListener (ParamID::fxRevSync, this);
}

void K1XAudioProcessorEditor::FxPage::parameterChanged (const juce::String& id, float)
{
    if (id != ParamID::fxRevSync)
        return;
    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<FxPage> (this)]
    {
        if (safe != nullptr)
            safe->bindPreKnob();
    });
}

void K1XAudioProcessorEditor::FxPage::bindPreKnob()
{
    if (knobs.size() < 5)
        return;
    auto* k = knobs[4];
    k->att.reset();
    const bool sync = proc.apvts.getRawParameterValue (ParamID::fxRevSync)->load() >= 0.5f;
    k->att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, sync ? ParamID::fxRevDiv : ParamID::fxRevPredelay, k->slider);
}

void K1XAudioProcessorEditor::FxPage::refreshMix()
{
    dryStrip.refresh();
    fxStrip.refresh();
}

void K1XAudioProcessorEditor::FxPage::resized()
{
    auto r = getLocalBounds();
    auto fx = r.removeFromTop (110);
    r.removeFromTop (12); // gap between FX knobs and EQ
    eq.setBounds (r);

    auto mix = fx.removeFromLeft (92);
    dryStrip.setBounds (mix.removeFromLeft (44));
    mix.removeFromLeft (4);
    fxStrip.setBounds (mix);

    auto dist = fx.removeFromLeft (90);
    dstOn.setBounds (dist.removeFromTop (22).reduced (4, 1));
    if (knobs.size() >= 1)
        knobs[0]->setBounds (dist);

    fx.removeFromLeft (4);
    auto delay = fx.removeFromLeft (220);
    auto delayTop = delay.removeFromTop (22);
    dlyOn.setBounds (delayTop.removeFromLeft (52).reduced (2, 1));
    dlySync.setBounds (delayTop.removeFromLeft (48).reduced (2, 1));
    dlyDivLabel.setBounds (delayTop.removeFromLeft (28));
    dlyDiv.setBounds (delayTop.removeFromLeft (64).reduced (0, 1));
    const int dlyW = delay.getWidth() / 3;
    if (knobs.size() >= 4)
    {
        knobs[1]->setBounds (delay.removeFromLeft (dlyW));
        knobs[2]->setBounds (delay.removeFromLeft (dlyW));
        knobs[3]->setBounds (delay);
    }

    fx.removeFromLeft (4);
    auto revTop = fx.removeFromTop (22);
    revOn.setBounds (revTop.removeFromLeft (48).reduced (2, 1));
    revSync.setBounds (revTop.removeFromLeft (48).reduced (2, 1));
    const int revW = juce::jmax (1, fx.getWidth() / 4);
    if (knobs.size() >= 8)
    {
        knobs[4]->setBounds (fx.removeFromLeft (revW));
        knobs[5]->setBounds (fx.removeFromLeft (revW));
        knobs[6]->setBounds (fx.removeFromLeft (revW));
        knobs[7]->setBounds (fx);
    }
}

void K1XAudioProcessorEditor::PanelLnF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                          float pos, float startAngle, float endAngle,
                                                          juce::Slider&)
{
    // Inset so the ticks stay inside the slider, including the 36 px EQ knobs.
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (1.f);
    const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
    if (d < 8.f)
        return;

    auto area = bounds.withSizeKeepingCentre (d, d);
    const auto c = area.getCentre();
    const float tickW = juce::jmax (1.f, d * 0.018f);

    g.setColour (kMetal.withAlpha (0.75f));
    constexpr int nTicks = 11;
    for (int i = 0; i < nTicks; ++i)
    {
        const float t = (float) i / (float) (nTicks - 1);
        const float a = startAngle + t * (endAngle - startAngle);
        const auto p0 = c.getPointOnCircumference (d * 0.48f, a);
        const auto p1 = c.getPointOnCircumference (d * 0.40f, a);
        g.drawLine (p0.x, p0.y, p1.x, p1.y, tickW);
    }

    const float bodyD = d * 0.72f;
    auto body = area.withSizeKeepingCentre (bodyD, bodyD);
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillEllipse (body.translated (0.f, d * 0.04f).expanded (d * 0.01f));
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (body.translated (0.f, d * 0.018f));

    g.setGradientFill (juce::ColourGradient (kChassis.brighter (0.18f), c.x, body.getY(),
                                              kDark, c.x, body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (kMetal.withAlpha (0.3f));
    g.drawEllipse (body.reduced (0.6f), juce::jmax (1.f, d * 0.012f));

    const float capD = bodyD * 0.5f;
    auto cap = area.withSizeKeepingCentre (capD, capD);
    g.setColour (kOrange);
    g.fillEllipse (cap);

    const float angle = startAngle + pos * (endAngle - startAngle);
    juce::Path pointer;
    pointer.startNewSubPath (c.getPointOnCircumference (bodyD * 0.32f, angle));
    pointer.lineTo (c.getPointOnCircumference (bodyD * 0.46f, angle));
    g.setColour (kWhite);
    g.strokePath (pointer, juce::PathStrokeType (juce::jmax (1.15f, d * 0.028f),
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    const float pip = juce::jmax (1.6f, d * 0.045f);
    const auto pipC = c.getPointOnCircumference (capD * 0.28f, angle);
    g.fillEllipse (pipC.x - pip * 0.5f, pipC.y - pip * 0.5f, pip, pip);
}

void K1XAudioProcessorEditor::PanelLnF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto bounds = b.getLocalBounds().toFloat().reduced (2.f);
    g.setColour (b.getToggleState() ? kOrange.withAlpha (0.85f) : kDark);
    g.fillRoundedRectangle (bounds, 3.f);
    g.setFont (boldFont (11.f));
    g.setColour (b.getToggleState() ? kWhite : kMetal);
    g.drawText (b.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, false);
}

void K1XAudioProcessorEditor::PanelLnF::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                                      int, int, int, int, juce::ComboBox&)
{
    auto r = juce::Rectangle<float> (0.f, 0.f, (float) width, (float) height);
    g.setColour (kDark);
    g.fillRoundedRectangle (r, 3.f);
    g.setColour (kMetal.withAlpha (0.35f));
    g.drawRoundedRectangle (r.reduced (0.5f), 3.f, 1.f);
}

juce::Font K1XAudioProcessorEditor::PanelLnF::getLabelFont (juce::Label&)
{
    return boldFont (11.f);
}

juce::Font K1XAudioProcessorEditor::PanelLnF::getComboBoxFont (juce::ComboBox&)
{
    return boldFont (11.f);
}

void K1XAudioProcessorEditor::WaveView::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (kDark);
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (kMetal.withAlpha (0.25f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.f, 1.f);

    if (samples.size() < 2)
        return;

    const float mid = r.getCentreY();
    const float amp = r.getHeight() * 0.42f;
    juce::Path p;
    const int n = (int) samples.size();
    for (int i = 0; i < n; ++i)
    {
        const float x = r.getX() + (float) i / (float) (n - 1) * r.getWidth();
        const float y = mid - samples[(size_t) i] * amp;
        if (i == 0)
            p.startNewSubPath (x, y);
        else
            p.lineTo (x, y);
    }
    g.setColour (kOrange);
    g.strokePath (p, juce::PathStrokeType (1.2f));
}

K1XAudioProcessorEditor::AboutBox::AboutBox()
{
    status.setJustificationType (juce::Justification::centred);
    status.setColour (juce::Label::textColourId, kMetal);
    status.setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (status);

    auto setupPlain = [] (juce::ToggleButton& b)
    {
        b.setClickingTogglesState (false);
    };
    setupPlain (checkBtn);
    setupPlain (closeBtn);
    checkBtn.onClick = [this] { checkForUpdate(); };
    closeBtn.onClick = [this] { setVisible (false); };
    addAndMakeVisible (checkBtn);
    addAndMakeVisible (closeBtn);

    releasesBtn.setColour (juce::HyperlinkButton::textColourId, kOrange);
    releasesBtn.setVisible (false);
    addAndMakeVisible (releasesBtn);
}

void K1XAudioProcessorEditor::AboutBox::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.55f));
    auto card = cardBounds().toFloat();
    g.setColour (kDark);
    g.fillRoundedRectangle (card, 6.f);
    g.setColour (kMetal.withAlpha (0.35f));
    g.drawRoundedRectangle (card.reduced (0.5f), 6.f, 1.f);

    auto r = cardBounds().reduced (16, 14);
    g.setColour (kMetal);
    g.setFont (boldFont (20.f));
    g.drawText ("K1X", r.removeFromTop (26), juce::Justification::centred, false);
    g.setFont (boldFont (12.f));
    g.drawText ("v" JucePlugin_VersionString, r.removeFromTop (18), juce::Justification::centred, false);
    g.setFont (boldFont (11.f));
    g.setColour (kMetal.withAlpha (0.8f));
    g.drawText ("Stupid Systems LLC", r.removeFromTop (16), juce::Justification::centred, false);
    g.drawText ("MIT", r.removeFromTop (16), juce::Justification::centred, false);
}

void K1XAudioProcessorEditor::AboutBox::resized()
{
    auto r = cardBounds().reduced (16, 14);
    r.removeFromTop (26 + 18 + 16 + 16 + 8);
    checkBtn.setBounds (r.removeFromTop (22).withSizeKeepingCentre (160, 22));
    r.removeFromTop (6);
    status.setBounds (r.removeFromTop (22));
    r.removeFromTop (4);
    releasesBtn.setBounds (r.removeFromTop (20).withSizeKeepingCentre (140, 20));
    closeBtn.setBounds (cardBounds().removeFromBottom (36).reduced (90, 8));
}

void K1XAudioProcessorEditor::AboutBox::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && ! cardBounds().contains (e.getPosition()))
        setVisible (false);
}

juce::Rectangle<int> K1XAudioProcessorEditor::AboutBox::cardBounds() const
{
    return getLocalBounds().withSizeKeepingCentre (320, 240);
}

void K1XAudioProcessorEditor::AboutBox::applyResult (const juce::String& text, bool hasUpdate)
{
    fetching.store (false);
    status.setText (text, juce::dontSendNotification);
    releasesBtn.setVisible (hasUpdate);
}

void K1XAudioProcessorEditor::AboutBox::checkForUpdate()
{
    if (fetching.exchange (true))
        return;
    status.setText ("Checking...", juce::dontSendNotification);
    releasesBtn.setVisible (false);
    juce::Component::SafePointer<AboutBox> safe (this);
    std::thread ([safe]
    {
        juce::String text;
        bool hasUpdate = false;
        juce::URL url ("https://api.github.com/repos/iamtew/K1X/tags");
        const auto opts = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                              .withExtraHeaders ("User-Agent: K1X\nAccept: application/vnd.github+json")
                              .withConnectionTimeoutMs (8000);
        if (auto stream = url.createInputStream (opts))
        {
            const auto parsed = juce::JSON::parse (stream->readEntireStreamAsString());
            if (auto* arr = parsed.getArray())
            {
                std::vector<std::string> names;
                names.reserve ((size_t) arr->size());
                for (const auto& v : *arr)
                    if (auto* obj = v.getDynamicObject())
                        names.push_back (obj->getProperty ("name").toString().toStdString());
                const auto newest = k1x::newestNewerTag (names, JucePlugin_VersionString);
                if (newest.empty())
                    text = "You are up to date.";
                else
                {
                    text = "Newer version available: " + juce::String (newest);
                    hasUpdate = true;
                }
            }
            else
                text = "Could not reach GitHub.";
        }
        else
            text = "Could not reach GitHub.";

        juce::MessageManager::callAsync ([safe, text, hasUpdate]
        {
            if (safe != nullptr)
                safe->applyResult (text, hasUpdate);
        });
    }).detach();
}

void K1XAudioProcessorEditor::PatchBar::setText (const juce::String& t)
{
    if (text == t)
        return;
    text = t;
    repaint();
}

void K1XAudioProcessorEditor::PatchBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (kDark);
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (kMetal.withAlpha (0.35f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.f, 1.f);

    auto left = r.removeFromLeft (18.f);
    auto right = r.removeFromRight (18.f);
    auto chev = [] (juce::Rectangle<float> box, bool back)
    {
        juce::Path p;
        const float cx = box.getCentreX();
        const float cy = box.getCentreY();
        if (back)
            p.addTriangle (cx + 3.f, cy - 5.f, cx - 4.f, cy, cx + 3.f, cy + 5.f);
        else
            p.addTriangle (cx - 3.f, cy - 5.f, cx + 4.f, cy, cx - 3.f, cy + 5.f);
        return p;
    };
    g.setColour (kMetal);
    g.fillPath (chev (left, true));
    g.fillPath (chev (right, false));
    g.setFont (boldFont (12.f));
    g.drawText (text, r.toNearestInt(), juce::Justification::centred, true);
}

void K1XAudioProcessorEditor::PatchBar::mouseUp (const juce::MouseEvent& e)
{
    if (! e.mouseWasClicked())
        return;
    const int w = getWidth();
    if (e.x < 20)
    {
        if (onStep)
            onStep (-1);
    }
    else if (e.x > w - 20)
    {
        if (onStep)
            onStep (1);
    }
    else if (onOpen)
        onOpen();
}

K1XAudioProcessorEditor::VolumeStrip::VolumeStrip (K1XAudioProcessorEditor& ed)
    : editor (ed)
{
    slider.setLookAndFeel (&lnf);
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    slider.setOpaque (false);
    slider.setColour (juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::trackColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::thumbColourId, kMetal);
    auto* p = dynamic_cast<juce::RangedAudioParameter*> (editor.proc.apvts.getParameter (ParamID::level));
    if (p != nullptr)
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
    sliderAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        editor.proc.apvts, ParamID::level, slider);
    addAndMakeVisible (slider);
}

K1XAudioProcessorEditor::VolumeStrip::~VolumeStrip()
{
    slider.setLookAndFeel (nullptr);
}

juce::Rectangle<float> K1XAudioProcessorEditor::VolumeStrip::faderArea() const
{
    auto r = getLocalBounds().toFloat();
    return r.removeFromLeft (r.getWidth() * 5.f / 9.f);
}

juce::Rectangle<float> K1XAudioProcessorEditor::VolumeStrip::scopeArea() const
{
    auto r = getLocalBounds().toFloat();
    r.removeFromLeft (r.getWidth() * 5.f / 9.f + 4.f);
    return r;
}

void K1XAudioProcessorEditor::VolumeStrip::resized()
{
    slider.setBounds (faderArea().toNearestInt());
}

void K1XAudioProcessorEditor::VolumeStrip::LnF::drawLinearSlider (juce::Graphics& g, int, int y, int, int h,
                                                                 float pos, float, float,
                                                                 juce::Slider::SliderStyle, juce::Slider&)
{
    const float by = (float) (y + h);
    juce::Path p;
    p.addTriangle (pos - 5.f, by, pos + 5.f, by, pos, by - 7.f);
    g.setColour (kDark);
    g.fillPath (p);
    g.setColour (kMetal);
    g.strokePath (p, juce::PathStrokeType (1.f));
}

void K1XAudioProcessorEditor::VolumeStrip::paint (juce::Graphics& g)
{
    auto fader = faderArea();
    auto scope = scopeArea();

    g.setColour (kDark);
    g.fillRoundedRectangle (fader, 3.f);
    g.fillRoundedRectangle (scope, 3.f);

    const float pos = (float) slider.valueToProportionOfLength (slider.getValue());
    auto fill = fader.withWidth (juce::jmax (2.f, fader.getWidth() * pos));
    g.setColour (clipped ? kOrange : kChassis.brighter (0.15f));
    g.fillRoundedRectangle (fill, 3.f);

    g.setColour (kMetal.withAlpha (0.35f));
    for (int i = 1; i < 8; ++i)
    {
        const float x = fader.getX() + fader.getWidth() * (float) i / 8.f;
        const float tickH = (i == 4) ? 6.f : 3.f;
        g.drawLine (x, fader.getBottom() - 2.f, x, fader.getBottom() - 2.f - tickH, 1.f);
    }

    const int n = juce::jlimit (2, 256, (int) scope.getWidth());
    float specDb[256];
    editor.proc.eqAnalyser().copyLogDb (specDb, n);
    juce::Path spec;
    spec.startNewSubPath (scope.getX(), scope.getBottom());
    for (int i = 0; i < n; ++i)
    {
        const float x01 = (float) i / (float) (n - 1);
        const float y01 = 1.f - juce::jlimit (0.f, 1.f, (specDb[i] + 72.f) / 72.f);
        spec.lineTo (scope.getX() + x01 * scope.getWidth(),
                     scope.getY() + y01 * scope.getHeight());
    }
    spec.lineTo (scope.getRight(), scope.getBottom());
    spec.closeSubPath();
    g.setColour ((clipped ? kOrange : kMetal).withAlpha (0.7f));
    g.fillPath (spec);
}

void K1XAudioProcessorEditor::VolumeStrip::refresh()
{
    if (editor.proc.outputMeter().takeClip())
        clipUntil = juce::Time::getMillisecondCounter() + 800;
    clipped = juce::Time::getMillisecondCounter() < clipUntil;
    repaint();
}

K1XAudioProcessorEditor::K1XAudioProcessorEditor (K1XAudioProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p), fxPage (p)
{
    setLookAndFeel (&panelLnF);
    setOpaque (true);
    panelLnF.setColour (juce::PopupMenu::backgroundColourId, kDark);
    panelLnF.setColour (juce::PopupMenu::textColourId, kMetal);
    panelLnF.setColour (juce::PopupMenu::highlightedBackgroundColourId, kOrange);
    panelLnF.setColour (juce::PopupMenu::highlightedTextColourId, kWhite);

    logo = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);

    content.addAndMakeVisible (wave);

    struct Spec { const char* id; const char* name; };
    static constexpr Spec kKnobs[] = {
        { ParamID::tune, "Tune" }, { ParamID::pitchDrop, "Drop" }, { ParamID::pitchDecay, "P.Decay" },
        { ParamID::click, "Click" }, { ParamID::punch, "Punch" }, { ParamID::decay, "Decay" },
        { ParamID::drive, "Drive" }, { ParamID::hpf, "HPF" }, { ParamID::tone, "Tone" },
        { ParamID::lo, "Lo" }, { ParamID::hi, "Hi" }
    };
    for (auto& s : kKnobs)
    {
        auto* k = knobs.add (new Knob (proc.apvts, s.id, s.name));
        content.addAndMakeVisible (k);
    }

    auto setupCombo = [this] (juce::Label& label, juce::ComboBox& box, const juce::String& name)
    {
        label.setText (name, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, kMetal);
        content.addAndMakeVisible (label);
        box.setColour (juce::ComboBox::textColourId, kMetal);
        box.setColour (juce::ComboBox::arrowColourId, kMetal);
        content.addAndMakeVisible (box);
    };
    setupCombo (styleLabel, styleBox, "Style");
    styleBox.addItem ("909", 1);
    styleBox.addItem ("808", 2);
    styleBox.addItem ("707", 3);
    styleBox.addItem ("Tight", 4);
    styleBox.addItem ("Hard", 5);
    styleAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.apvts, ParamID::style, styleBox);

    setupCombo (lengthLabel, lengthBox, "Length");
    lengthBox.addItem ("Free", 1);
    lengthBox.addItem ("1/4", 2);
    lengthBox.addItem ("1/8", 3);
    lengthBox.addItem ("1/16", 4);
    lengthAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.apvts, ParamID::length, lengthBox);

    patchBar.onStep = [this] (int d) { cyclePatch (d); };
    patchBar.onOpen = [this] { openPatchMenu(); };
    content.addAndMakeVisible (patchBar);
    refreshPatchName();
    pageBar.setText (kPageNames[0]);
    pageBar.onStep = [this] (int d) { cyclePage (d); };
    pageBar.onOpen = [this] { openPageMenu(); };
    content.addAndMakeVisible (pageBar);
    content.addAndMakeVisible (volumeStrip);

    standalone = juce::PluginHostType::getPluginLoadedAs() == juce::AudioProcessor::wrapperType_Standalone;
    playBtn.setClickingTogglesState (false);
    playBtn.onClick = [this] { proc.requestTrigger(); };
    exportBtn.setClickingTogglesState (false);
    exportBtn.onClick = [this] { exportWav(); };
    content.addAndMakeVisible (playBtn);
    content.addAndMakeVisible (exportBtn);
    playBtn.setVisible (standalone);
    exportBtn.setVisible (standalone);

    content.addAndMakeVisible (fxPage);
    fxPage.setVisible (false);

    content.addChildComponent (aboutBox);
    aboutBox.setVisible (false);

    for (auto* id : { ParamID::tune, ParamID::pitchDrop, ParamID::pitchDecay, ParamID::click,
                      ParamID::punch, ParamID::decay, ParamID::drive, ParamID::hpf, ParamID::tone,
                      ParamID::lo, ParamID::hi, ParamID::level, ParamID::length, ParamID::style })
        proc.apvts.addParameterListener (id, this);

    baseW = kEditorW;
    baseH = kEditorH;
    content.setSize (baseW, baseH);
    addAndMakeVisible (content);

    // Drag the corner to scale (40%..150%), aspect ratio locked. The scale is saved with the session.
    // Read it first: setResizeLimits clamps the still-empty editor and resized() overwrites uiScale.
    const float s = juce::jlimit (0.4f, 1.5f, proc.uiScale);
    setResizable (true, true);
    setResizeLimits (baseW * 2 / 5, baseH * 2 / 5, baseW * 3 / 2, baseH * 3 / 2);
    getConstrainer()->setFixedAspectRatio ((double) baseW / (double) baseH);
    setSize (juce::roundToInt ((float) baseW * s), juce::roundToInt ((float) baseH * s));
    startTimerHz (15);
}

K1XAudioProcessorEditor::~K1XAudioProcessorEditor()
{
    stopTimer();
    for (auto* id : { ParamID::tune, ParamID::pitchDrop, ParamID::pitchDecay, ParamID::click,
                      ParamID::punch, ParamID::decay, ParamID::drive, ParamID::hpf, ParamID::tone,
                      ParamID::lo, ParamID::hi, ParamID::level, ParamID::length, ParamID::style })
        proc.apvts.removeParameterListener (id, this);
    setLookAndFeel (nullptr);
}

void K1XAudioProcessorEditor::parameterChanged (const juce::String&, float)
{
    waveDirty = true;
}

void K1XAudioProcessorEditor::timerCallback()
{
    if (waveDirty)
        rebuildWave();
    refreshPatchName();
    volumeStrip.refresh();
    if (fxPage.isVisible())
        fxPage.refreshMix();
}

void K1XAudioProcessorEditor::rebuildWave()
{
    waveDirty = false;
    const int n = 1024;
    std::vector<float> full (44100);
    proc.renderShot (full.data(), (int) full.size(), 44100.0);

    int last = (int) full.size() - 1;
    while (last > 64 && std::abs (full[(size_t) last]) < 0.002f)
        --last;
    last = juce::jmax (64, last);

    wave.samples.resize ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        const int src = (int) ((double) i / (double) (n - 1) * (double) last);
        wave.samples[(size_t) i] = juce::jlimit (-1.f, 1.f, full[(size_t) src]);
    }
    wave.repaint();
}

void K1XAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (kChassis);  // covers the sub-pixel edge the scaled content may leave
}

void K1XAudioProcessorEditor::resized()
{
    if (baseW == 0)
        return;  // called before the constructor finished
    const float s = (float) getWidth() / (float) baseW;
    content.setTransform (juce::AffineTransform::scale (s));
    proc.uiScale = s;
}

void K1XAudioProcessorEditor::paintContent (juce::Graphics& g)
{
    g.fillAll (kChassis);

    auto r = content.getLocalBounds().reduced (kPad);
    auto title = r.removeFromTop (kTitleH);
    if (logo.isValid())
    {
        auto logoR = title.removeFromLeft (kTitleH).reduced (2);
        g.drawImage (logo, logoR.toFloat(), juce::RectanglePlacement::centred);
        title.removeFromLeft (6);
    }
    auto nameCol = title.removeFromLeft (90);
    g.setColour (kMetal);
    g.setFont (boldFont (16.f));
    g.drawText ("K1X", nameCol.removeFromTop (22), juce::Justification::centredLeft, false);
    g.setFont (boldFont (10.f));
    g.drawText ("v" JucePlugin_VersionString, nameCol, juce::Justification::centredLeft, false);

    g.setColour (kChassis.brighter (0.08f));
    g.drawRoundedRectangle (content.getLocalBounds().toFloat().reduced (1.5f), 2.f, 2.f);
}

void K1XAudioProcessorEditor::layoutContent()
{
    auto r = content.getLocalBounds().reduced (kPad);
    auto title = r.removeFromTop (kTitleH);
    title.removeFromLeft (kTitleHitW);
    volumeStrip.setBounds (title.removeFromRight (240).reduced (2, 4));
    pageBar.setBounds (title.removeFromLeft (88).reduced (4, 4));
    patchBar.setBounds (title.reduced (4, 4));

    auto tools = r.removeFromTop (28);
    styleLabel.setBounds (tools.removeFromLeft (44));
    styleBox.setBounds (tools.removeFromLeft (80).reduced (0, 2));
    tools.removeFromLeft (8);
    lengthLabel.setBounds (tools.removeFromLeft (52));
    lengthBox.setBounds (tools.removeFromLeft (80).reduced (0, 2));
    if (standalone)
    {
        tools.removeFromLeft (8);
        playBtn.setBounds (tools.removeFromLeft (64).reduced (0, 2));
        exportBtn.setBounds (tools.removeFromLeft (96).reduced (0, 2));
    }

    fxPage.setBounds (r);
    wave.setBounds (r.removeFromTop (110).reduced (0, 4));
    r.removeFromTop (6);
    const int cols = 6;
    const int rows = 2;
    const int cellW = r.getWidth() / cols;
    const int cellH = r.getHeight() / rows;
    for (int i = 0; i < knobs.size(); ++i)
        knobs[i]->setBounds (r.getX() + (i % cols) * cellW,
                             r.getY() + (i / cols) * cellH,
                             cellW, cellH);

    aboutBox.setBounds (content.getLocalBounds());
    applyPageVisibility();
}

juce::Rectangle<int> K1XAudioProcessorEditor::titleHitBounds() const
{
    return { kPad, kPad, kTitleHitW, kTitleH };
}

void K1XAudioProcessorEditor::setEditorPage (int page)
{
    editorPage = juce::jlimit (0, kPageCount - 1, page);
    pageBar.setText (kPageNames[editorPage]);
    applyPageVisibility();
}

void K1XAudioProcessorEditor::cyclePage (int delta)
{
    setEditorPage ((editorPage + delta + kPageCount) % kPageCount);
}

void K1XAudioProcessorEditor::openPageMenu()
{
    juce::PopupMenu m;
    for (int i = 0; i < kPageCount; ++i)
        m.addItem (i + 1, kPageNames[i], true, i == editorPage);

    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (pageBar),
                     [safe] (int result)
                     {
                         if (safe == nullptr || result <= 0)
                             return;
                         safe->setEditorPage (result - 1);
                     });
}

void K1XAudioProcessorEditor::applyPageVisibility()
{
    const bool main = editorPage == 0;
    wave.setVisible (main);
    for (auto* k : knobs)
        k->setVisible (main);
    fxPage.setVisible (! main);
}

void K1XAudioProcessorEditor::contentMouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && titleHitBounds().contains (e.getPosition()))
        openAbout();
}

void K1XAudioProcessorEditor::refreshPatchName()
{
    const auto name = proc.patchName();
    if (const auto* preset = k1x::findFactoryPreset (name.toRawUTF8()))
        patchBar.setText (juce::String (preset->category) + " / " + name);
    else
        patchBar.setText (name);
}

void K1XAudioProcessorEditor::cyclePatch (int delta)
{
    const auto names = proc.patchNames();
    if (names.isEmpty())
        return;
    const auto cur = proc.patchName();
    int idx = names.indexOf (cur);
    if (idx < 0)
        idx = delta > 0 ? 0 : names.size() - 1;
    else
        idx = (idx + delta + names.size()) % names.size();
    proc.loadPatchByName (names[idx]);
    refreshPatchName();
}

void K1XAudioProcessorEditor::openPatchMenu()
{
    juce::PopupMenu m;
    const auto cur = proc.patchName();
    m.addItem (kMenuInit, "Init Patch", true, cur == "Init Patch");
    m.addSeparator();
    m.addItem (kMenuSave, "Save");
    m.addItem (kMenuSaveAs, "Save As...");
    m.addItem (kMenuExport, "Export...");
    m.addItem (kMenuImport, "Import...");
    m.addSeparator();

    juce::String cat;
    juce::PopupMenu sub;
    for (int i = 0; i < k1x::factoryPresetCount; ++i)
    {
        const auto& preset = k1x::factoryPresets[i];
        const juce::String next (preset.category);
        if (next != cat)
        {
            if (sub.getNumItems() > 0)
                m.addSubMenu (cat, sub);
            sub = juce::PopupMenu();
            cat = next;
        }
        sub.addItem (kMenuFactory + i, preset.name, true, cur == preset.name);
    }
    if (sub.getNumItems() > 0)
        m.addSubMenu (cat, sub);

    juce::StringArray userOnly;
    for (const auto& name : proc.patchNames())
        if (k1x::findFactoryPreset (name.toRawUTF8()) == nullptr)
            userOnly.add (name);
    if (userOnly.size() > 0)
    {
        m.addSeparator();
        for (int i = 0; i < userOnly.size(); ++i)
            m.addItem (kMenuUser + i, userOnly[i], true, userOnly[i] == cur);
    }

    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (patchBar),
                     [safe, userOnly] (int result)
                     {
                         if (safe == nullptr || result <= 0)
                             return;
                         if (result == kMenuInit)
                             safe->proc.initPatch();
                         else if (result == kMenuSave)
                         {
                             if (! safe->proc.savePatch())
                                 safe->chooseSaveAs();
                         }
                         else if (result == kMenuSaveAs)
                             safe->chooseSaveAs();
                         else if (result == kMenuExport)
                             safe->chooseExport();
                         else if (result == kMenuImport)
                             safe->chooseImport();
                         else if (result >= kMenuUser)
                         {
                             const int i = result - kMenuUser;
                             if (juce::isPositiveAndBelow (i, userOnly.size()))
                                 safe->proc.loadPatchByName (userOnly[i]);
                         }
                         else if (result >= kMenuFactory)
                         {
                             const int i = result - kMenuFactory;
                             if (juce::isPositiveAndBelow (i, k1x::factoryPresetCount))
                                 safe->proc.loadPatchByName (k1x::factoryPresets[i].name);
                         }
                         safe->refreshPatchName();
                     });
}

void K1XAudioProcessorEditor::chooseSaveAs()
{
    const auto start = proc.patchesDir().getChildFile ("Patch.k1xp");
    chooser = std::make_unique<juce::FileChooser> ("Save Patch", start, "*.k1xp");
    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode
                              | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe] (const juce::FileChooser& c)
                          {
                              if (safe == nullptr)
                                  return;
                              auto file = c.getResult();
                              if (file.getFullPathName().isEmpty())
                                  return;
                              safe->proc.savePatchAs (file.getFileNameWithoutExtension());
                              safe->refreshPatchName();
                          });
}

void K1XAudioProcessorEditor::chooseExport()
{
    const auto start = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                           .getChildFile ("K1X Patch.k1xp");
    chooser = std::make_unique<juce::FileChooser> ("Export Patch", start, "*.k1xp");
    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode
                              | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe] (const juce::FileChooser& c)
                          {
                              if (safe == nullptr)
                                  return;
                              auto file = c.getResult();
                              if (file.getFullPathName().isEmpty())
                                  return;
                              safe->proc.exportPatch (file);
                          });
}

void K1XAudioProcessorEditor::chooseImport()
{
    chooser = std::make_unique<juce::FileChooser> (
        "Import Patch",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        "*.k1xp");
    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::openMode
                              | juce::FileBrowserComponent::canSelectFiles,
                          [safe] (const juce::FileChooser& c)
                          {
                              if (safe == nullptr)
                                  return;
                              auto file = c.getResult();
                              if (! file.existsAsFile())
                                  return;
                              safe->proc.importPatch (file);
                              safe->refreshPatchName();
                          });
}

void K1XAudioProcessorEditor::openAbout()
{
    aboutBox.setVisible (true);
    aboutBox.toFront (true);
}

void K1XAudioProcessorEditor::exportWav()
{
    chooser = std::make_unique<juce::FileChooser> ("Export kick",
                                                   juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                                                       .getChildFile ("K1X.wav"),
                                                   "*.wav");
    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode
                              | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [safe] (const juce::FileChooser& c)
                          {
                              if (safe == nullptr)
                                  return;
                              auto file = c.getResult();
                              if (file.getFullPathName().isEmpty())
                                  return;
                              if (! file.hasFileExtension (".wav"))
                                  file = file.withFileExtension (".wav");

                              const double sr = 44100.0;
                              const int n = 44100 * 4; // kick + ~2 s FX tail
                              juce::AudioBuffer<float> buf (2, n);
                              safe->proc.renderWithPost (buf, sr);

                              juce::WavAudioFormat fmt;
                              std::unique_ptr<juce::OutputStream> out (file.createOutputStream());
                              if (out == nullptr)
                                  return;
                              const auto opts = juce::AudioFormatWriterOptions{}
                                                    .withSampleRate (sr)
                                                    .withNumChannels (2)
                                                    .withBitsPerSample (16);
                              if (auto w = fmt.createWriterFor (out, opts))
                                  w->writeFromAudioSampleBuffer (buf, 0, n);
                          });
}
