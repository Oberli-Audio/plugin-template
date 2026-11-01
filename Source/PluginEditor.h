#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "oberli/Buttons.h"
#include "oberli/Knob.h"
#include "oberli/Meters.h"
#include "oberli/StripEditor.h"

/*  {{Product}} - the editor.

    The template's editor is the family's Strip frame (Source/oberli/StripEditor.h: the header
    with the mark, the name, the preset pill and the menu; the preset drawer; Save preset; About
    on the back; the GUI scale) around a skeleton body: one CONTROLS group with a 48 px knob per
    float parameter, a pill per switch, a segmented control per choice of up to five, and the
    output column last on the right (a hairline, Mix at 64 px when the plugin has one, the level
    meter with its CLIP LED). plugin-ui-bob builds the real body at G2 from the parameter table
    (groups, core controls, the hero) and replaces this file's layout; the frame stays. */
class {{ProductId}}AudioProcessorEditor final : public oberli::ui::StripEditor
{
public:
    explicit {{ProductId}}AudioProcessorEditor ({{ProductId}}AudioProcessor&);
    ~{{ProductId}}AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;

    /** Shows a knob's drag readout without a mouse (the snapshot tool). False for an unknown ID. */
    bool previewDrag (const juce::String& parameterId) override;

private:
    void layoutBody (juce::Rectangle<int> area) override;
    void advanceBody (double now, float dt) override;
    void presetRecalled (double now) override;

    {{ProductId}}AudioProcessor& audioProcessor;   // not `processor`: AudioProcessorEditor has one (Apple clang -Wshadow)

    std::vector<std::unique_ptr<oberli::ui::ParameterKnob>> knobs;
    std::unique_ptr<oberli::ui::ParameterKnob> mixKnob;
    std::vector<std::unique_ptr<oberli::ui::PillToggle>> toggles;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> toggleAttachments;
    std::vector<std::unique_ptr<oberli::ui::SegmentedControl>> segments;
    oberli::ui::LevelMeter meter;

    juce::Rectangle<int> controlsTitle, outputTitle, outputRule;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR ({{ProductId}}AudioProcessorEditor)
};
