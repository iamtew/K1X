#pragma once

#include "PluginProcessor.h"

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

    struct Knob : public juce::Component
    {
        Knob (juce::AudioProcessorValueTreeState& st, const juce::String& id, const juce::String& name);
        void resized() override;

        juce::Label label;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    };

    PanelLnF panelLnF;
    WaveView wave;
    juce::OwnedArray<Knob> knobs;
    juce::Label lengthLabel;
    juce::ComboBox lengthBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lengthAtt;
    PatchBar patchBar;
    juce::ToggleButton playBtn { "Play" };
    juce::ToggleButton exportBtn { "Export WAV" };
    AboutBox aboutBox;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Image logo;
    bool waveDirty = true;
    bool standalone = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (K1XAudioProcessorEditor)
};
