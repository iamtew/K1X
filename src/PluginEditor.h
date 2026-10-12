#pragma once

#include "PluginProcessor.h"
#include "ui/EqPage.h"

#include <atomic>
#include <functional>
#include <vector>

class K1XAudioProcessorEditor : public juce::AudioProcessorEditor,
                                private juce::Timer,
                                private juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit K1XAudioProcessorEditor (K1XAudioProcessor&);
    ~K1XAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void parameterChanged (const juce::String&, float) override;
    void rebuildWave();
    juce::Rectangle<int> titleHitBounds() const;
    void openAbout();
    void exportWav();
    void refreshPatchName();
    void openPatchMenu();
    void cyclePatch (int delta);
    void chooseSaveAs();
    void chooseExport();
    void chooseImport();
    void setEditorPage (int page);
    void openPageMenu();
    void cyclePage (int delta);
    void applyPageVisibility();

    K1XAudioProcessor& proc;

    struct PanelLnF : public juce::LookAndFeel_V4
    {
        void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                               float startAngle, float endAngle, juce::Slider&) override;
        void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
        void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
        juce::Font getLabelFont (juce::Label&) override;
        juce::Font getComboBoxFont (juce::ComboBox&) override;
    };

    struct AboutBox : public juce::Component
    {
        AboutBox();
        void paint (juce::Graphics& g) override;
        void resized() override;
        void mouseUp (const juce::MouseEvent& e) override;
        void checkForUpdate();
        void applyResult (const juce::String& text, bool hasUpdate);
        juce::Rectangle<int> cardBounds() const;

        juce::Label status;
        juce::ToggleButton checkBtn { "Check for update" };
        juce::HyperlinkButton releasesBtn {
            "Open releases",
            juce::URL ("https://github.com/iamtew/K1X/releases/latest")
        };
        juce::ToggleButton closeBtn { "Close" };
        std::atomic<bool> fetching { false };
    };

    struct WaveView : public juce::Component
    {
        void paint (juce::Graphics& g) override;
        std::vector<float> samples;
    };

    struct PatchBar : public juce::Component
    {
        void paint (juce::Graphics& g) override;
        void mouseUp (const juce::MouseEvent& e) override;
        void setText (const juce::String& t);

        juce::String text;
        std::function<void (int)> onStep;
        std::function<void()> onOpen;
    };

    struct VolumeStrip : public juce::Component
    {
        explicit VolumeStrip (K1XAudioProcessorEditor& ed);
        ~VolumeStrip() override;
        void resized() override;
        void paint (juce::Graphics& g) override;
        void refresh();
        juce::Rectangle<float> faderArea() const;
        juce::Rectangle<float> scopeArea() const;

        struct LnF : public juce::LookAndFeel_V4
        {
            void drawLinearSlider (juce::Graphics&, int, int, int, int, float pos,
                                   float, float, juce::Slider::SliderStyle, juce::Slider&) override;
        };

        K1XAudioProcessorEditor& editor;
        LnF lnf;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAtt;
        bool clipped = false;
        juce::uint32 clipUntil = 0;
    };

    struct Knob : public juce::Component
    {
        Knob (juce::AudioProcessorValueTreeState& st, const juce::String& id, const juce::String& name);
        void resized() override;

        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };

    // Effects send + post EQ. Shares one editor page with Main.
    struct FxPage : public juce::Component,
                    private juce::AudioProcessorValueTreeState::Listener
    {
        explicit FxPage (K1XAudioProcessor& p);
        ~FxPage() override;
        void resized() override;
        void parameterChanged (const juce::String&, float) override;
        void bindPreKnob();

        K1XAudioProcessor& proc;
        juce::OwnedArray<Knob> knobs;
        juce::ToggleButton dstOn { "Dist" };
        juce::ToggleButton dlyOn { "Delay" };
        juce::ToggleButton dlySync { "Sync" };
        juce::ToggleButton revOn { "Rev" };
        juce::ToggleButton revSync { "Sync" };
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dstOnAtt, dlyOnAtt, dlySyncAtt, revOnAtt, revSyncAtt;
        juce::ComboBox dlyDiv;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlyDivAtt;
        juce::Label dlyDivLabel;
        EqPage eq;
    };

    PanelLnF panelLnF;
    WaveView wave;
    juce::OwnedArray<Knob> knobs;
    juce::Label styleLabel;
    juce::ComboBox styleBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> styleAtt;
    juce::Label lengthLabel;
    juce::ComboBox lengthBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lengthAtt;
    PatchBar patchBar;
    PatchBar pageBar;
    VolumeStrip volumeStrip { *this };
    juce::ToggleButton playBtn { "Play" };
    juce::ToggleButton exportBtn { "Export WAV" };
    FxPage fxPage;
    AboutBox aboutBox;
    int editorPage = 0;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Image logo;
    bool waveDirty = true;
    bool standalone = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (K1XAudioProcessorEditor)
};
