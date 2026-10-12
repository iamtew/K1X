#include "PluginEditor.h"
#include "ui/VersionCompare.h"

#include <cmath>
#include <thread>
#include <vector>

namespace
{
constexpr int kEditorW = 720;
constexpr int kEditorH = 460;
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
constexpr int kMenuFiles = 100;
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

void K1XAudioProcessorEditor::PanelLnF::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                                          float pos, float startAngle, float endAngle,
                                                          juce::Slider&)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (3.f);
    const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
    auto rc = bounds.withSizeKeepingCentre (d, d);

    g.setColour (kDark);
    g.fillEllipse (rc);

    auto cap = rc.reduced (d * 0.16f);
    g.setColour (kMetal);
    g.fillEllipse (cap);
    g.setColour (kWhite);
    g.fillEllipse (cap.translated (-d * 0.06f, -d * 0.08f).withSizeKeepingCentre (cap.getWidth() * 0.45f,
                                                                                 cap.getHeight() * 0.35f));

    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto c = cap.getCentre();
    const auto tip = c.getPointOnCircumference (cap.getWidth() * 0.42f, angle);
    g.setColour (kDark);
    g.drawLine (c.x, c.y, tip.x, tip.y, 2.2f);
    g.fillEllipse (c.x - 2.5f, c.y - 2.5f, 5.f, 5.f);
}

void K1XAudioProcessorEditor::PanelLnF::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto bounds = b.getLocalBounds().toFloat().reduced (2.f);
    g.setColour (kDark);
    g.fillRoundedRectangle (bounds, 3.f);
    g.setFont (boldFont (11.f));
    g.setColour (kMetal);
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
    g.drawText (text, r.toNearestInt(), juce::Justification::centred, false);
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

K1XAudioProcessorEditor::K1XAudioProcessorEditor (K1XAudioProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&panelLnF);
    setOpaque (true);
    panelLnF.setColour (juce::PopupMenu::backgroundColourId, kDark);
    panelLnF.setColour (juce::PopupMenu::textColourId, kMetal);
    panelLnF.setColour (juce::PopupMenu::highlightedBackgroundColourId, kOrange);
    panelLnF.setColour (juce::PopupMenu::highlightedTextColourId, kWhite);

    logo = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);

    addAndMakeVisible (wave);

    struct Spec { const char* id; const char* name; };
    static constexpr Spec kKnobs[] = {
        { ParamID::tune, "Tune" }, { ParamID::pitchDrop, "Drop" }, { ParamID::pitchDecay, "P.Decay" },
        { ParamID::click, "Click" }, { ParamID::punch, "Punch" }, { ParamID::decay, "Decay" },
        { ParamID::drive, "Drive" }, { ParamID::hpf, "HPF" }, { ParamID::tone, "Tone" },
        { ParamID::lo, "Lo" }, { ParamID::hi, "Hi" }, { ParamID::level, "Level" }
    };
    for (auto& s : kKnobs)
    {
        auto* k = knobs.add (new Knob (proc.apvts, s.id, s.name));
        addAndMakeVisible (k);
    }

    lengthLabel.setText ("Length", juce::dontSendNotification);
    lengthLabel.setJustificationType (juce::Justification::centred);
    lengthLabel.setColour (juce::Label::textColourId, kMetal);
    addAndMakeVisible (lengthLabel);
    lengthBox.setColour (juce::ComboBox::textColourId, kMetal);
    lengthBox.setColour (juce::ComboBox::arrowColourId, kMetal);
    lengthBox.addItem ("Free", 1);
    lengthBox.addItem ("1/4", 2);
    lengthBox.addItem ("1/8", 3);
    lengthBox.addItem ("1/16", 4);
    addAndMakeVisible (lengthBox);
    lengthAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.apvts, ParamID::length, lengthBox);

    patchBar.onStep = [this] (int d) { cyclePatch (d); };
    patchBar.onOpen = [this] { openPatchMenu(); };
    addAndMakeVisible (patchBar);
    refreshPatchName();

    standalone = juce::PluginHostType::getPluginLoadedAs() == juce::AudioProcessor::wrapperType_Standalone;
    playBtn.setClickingTogglesState (false);
    playBtn.onClick = [this] { proc.requestTrigger(); };
    exportBtn.setClickingTogglesState (false);
    exportBtn.onClick = [this] { exportWav(); };
    addAndMakeVisible (playBtn);
    addAndMakeVisible (exportBtn);
    playBtn.setVisible (standalone);
    exportBtn.setVisible (standalone);

    addChildComponent (aboutBox);
    aboutBox.setVisible (false);

    for (auto* id : { ParamID::tune, ParamID::pitchDrop, ParamID::pitchDecay, ParamID::click,
                      ParamID::punch, ParamID::decay, ParamID::drive, ParamID::hpf, ParamID::tone,
                      ParamID::lo, ParamID::hi, ParamID::level, ParamID::length })
        proc.apvts.addParameterListener (id, this);

    setSize (kEditorW, kEditorH);
    startTimerHz (8);
}

K1XAudioProcessorEditor::~K1XAudioProcessorEditor()
{
    stopTimer();
    for (auto* id : { ParamID::tune, ParamID::pitchDrop, ParamID::pitchDecay, ParamID::click,
                      ParamID::punch, ParamID::decay, ParamID::drive, ParamID::hpf, ParamID::tone,
                      ParamID::lo, ParamID::hi, ParamID::level, ParamID::length })
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
    g.fillAll (kChassis);

    auto r = getLocalBounds().reduced (kPad);
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
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.5f), 2.f, 2.f);
}

void K1XAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (kPad);
    auto title = r.removeFromTop (kTitleH);
    title.removeFromLeft (kTitleHitW);
    patchBar.setBounds (title.removeFromRight (juce::jmin (220, title.getWidth())).reduced (4, 4));
    wave.setBounds (r.removeFromTop (110).reduced (0, 4));

    auto tools = r.removeFromTop (28);
    lengthLabel.setBounds (tools.removeFromLeft (52));
    lengthBox.setBounds (tools.removeFromLeft (80).reduced (0, 2));
    if (standalone)
    {
        tools.removeFromLeft (8);
        playBtn.setBounds (tools.removeFromLeft (64).reduced (0, 2));
        exportBtn.setBounds (tools.removeFromLeft (96).reduced (0, 2));
    }

    r.removeFromTop (6);
    const int cols = 6;
    const int rows = 2;
    const int cellW = r.getWidth() / cols;
    const int cellH = r.getHeight() / rows;
    for (int i = 0; i < knobs.size(); ++i)
        knobs[i]->setBounds (r.getX() + (i % cols) * cellW,
                             r.getY() + (i / cols) * cellH,
                             cellW, cellH);

    aboutBox.setBounds (getLocalBounds());
}

juce::Rectangle<int> K1XAudioProcessorEditor::titleHitBounds() const
{
    return { kPad, kPad, kTitleHitW, kTitleH };
}

void K1XAudioProcessorEditor::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && titleHitBounds().contains (e.getPosition()))
        openAbout();
}

void K1XAudioProcessorEditor::refreshPatchName()
{
    patchBar.setText (proc.patchName());
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
    const auto names = proc.patchNames();
    const auto cur = proc.patchName();
    m.addItem (kMenuInit, "Init Patch", true, cur == "Init Patch");
    m.addSeparator();
    m.addItem (kMenuSave, "Save");
    m.addItem (kMenuSaveAs, "Save As...");
    m.addItem (kMenuExport, "Export...");
    m.addItem (kMenuImport, "Import...");
    if (names.size() > 0)
    {
        m.addSeparator();
        for (int i = 0; i < names.size(); ++i)
            m.addItem (kMenuFiles + i, names[i], true, names[i] == cur);
    }

    juce::Component::SafePointer<K1XAudioProcessorEditor> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (patchBar),
                     [safe, names] (int result)
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
                         else if (result >= kMenuFiles)
                         {
                             const int i = result - kMenuFiles;
                             if (juce::isPositiveAndBelow (i, names.size()))
                                 safe->proc.loadPatchByName (names[i]);
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
                              const int n = 44100 * 2;
                              juce::AudioBuffer<float> buf (1, n);
                              safe->proc.renderShot (buf.getWritePointer (0), n, sr);

                              juce::WavAudioFormat fmt;
                              std::unique_ptr<juce::OutputStream> out (file.createOutputStream());
                              if (out == nullptr)
                                  return;
                              const auto opts = juce::AudioFormatWriterOptions{}
                                                    .withSampleRate (sr)
                                                    .withNumChannels (1)
                                                    .withBitsPerSample (16);
                              if (auto w = fmt.createWriterFor (out, opts))
                                  w->writeFromAudioSampleBuffer (buf, 0, n);
                          });
}
