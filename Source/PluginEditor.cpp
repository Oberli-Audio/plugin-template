#include "PluginEditor.h"
#include "BuildInfo.h"
#include "Params.h"
#include "State.h"

#include <ProductData.h>

namespace
{
using namespace oberli::ui;

constexpr int kGroupGap = Tokens::space6;   // between groups
constexpr int kKnobGap = Tokens::space5;    // between knobs in a row
constexpr int kTitleHeight = 14 + Tokens::space3;

/** A document compiled into the plugin (CMakeLists.txt: ProductData), by its file name. */
juce::String embedded (const char* fileName)
{
    int size = 0;
    for (int i = 0; i < ProductData::namedResourceListSize; ++i)
        if (juce::String (ProductData::getNamedResourceOriginalFilename (ProductData::namedResourceList[i])) == fileName)
            if (const auto* data = ProductData::getNamedResource (ProductData::namedResourceList[i], size))
                return juce::String::fromUTF8 (data, size);
    return {};
}

StripEditor::Info infoFor()
{
    StripEditor::Info info;
    info.productName = JucePlugin_Name;
    info.version = PRODUCT_VERSION;
    info.what = PRODUCT_IS_INSTRUMENT ? "An instrument by Oberli Audio. Drag it onto a MIDI track and play."
                                      : "An effect by Oberli Audio. Put it on a track and turn it.";
    info.licences = embedded ("LICENSE") + "\n\n" + embedded ("THIRD_PARTY_NOTICES.md");
    info.soundVersion = {{bundle}}::State::kDspVersion;
    return info;
}

juce::String shortName (const juce::RangedAudioParameter& p)
{
    return p.getName (12);
}
} // namespace

{{ProductId}}AudioProcessorEditor::{{ProductId}}AudioProcessorEditor ({{ProductId}}AudioProcessor& p)
    : StripEditor (p, p.presetHost(), infoFor()),
      audioProcessor (p),
      meter (p.outputPeaks(), LevelMeter::Led::clip)
{
    auto& apvts = audioProcessor.parameters();

    for (auto* param : audioProcessor.getParameters())
    {
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param);
        if (ranged == nullptr)
            continue;

        const auto id = ranged->paramID;
        const juce::String tip (Params::tooltipFor (id.toRawUTF8()));

        if (dynamic_cast<juce::AudioParameterBool*> (ranged) != nullptr)
        {
            auto t = std::make_unique<PillToggle> (shortName (*ranged));
            t->setTooltip (tip);
            toggleAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, id, *t));
            addAndMakeVisible (*t);
            toggles.push_back (std::move (t));
        }
        else if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (ranged); choice != nullptr && choice->choices.size() <= 5)
        {
            auto s = std::make_unique<SegmentedControl> (choice->choices);
            s->attach (*ranged);
            s->setTooltip (tip);
            addAndMakeVisible (*s);
            segments.push_back (std::move (s));
        }
        else
        {
            const auto isMix = id == "mix";
            auto k = std::make_unique<ParameterKnob> (apvts, id, shortName (*ranged), isMix ? Tokens::knobLg : Tokens::knobMd);
            k->knob().setTooltip (tip);
            k->knob().setHelpText (tip);
            addAndMakeVisible (*k);
            if (isMix)
                mixKnob = std::move (k);
            else
                knobs.push_back (std::move (k));
        }
    }

    addAndMakeVisible (meter);

    // The body at 100 %: the CONTROLS group, then the output column.
    auto controlsWidth = 0;
    for (auto& k : knobs) controlsWidth += k->getPreferredWidth() + kKnobGap;
    for (auto& s : segments) controlsWidth += s->getPreferredWidth() + kKnobGap;
    controlsWidth = juce::jmax (160, controlsWidth - kKnobGap);
    const auto outputWidth = juce::jmax (meter.getPreferredWidth(), mixKnob != nullptr ? mixKnob->getPreferredWidth() : 0);
    const auto outputHeight = kTitleHeight + (mixKnob != nullptr ? mixKnob->getPreferredHeight() + Tokens::space5 : 0)
                            + LevelMeter::kHeight + (toggles.empty() ? 0 : Tokens::space4 + 28 * (int) toggles.size());
    setBodySize (controlsWidth + kGroupGap + 1 + kGroupGap + outputWidth,
                 juce::jmax (outputHeight, kTitleHeight + 48 + 36));

    audioProcessor.presetLibrary().writeFactoryFolder();   // the bank as files, once per version
}

{{ProductId}}AudioProcessorEditor::~{{ProductId}}AudioProcessorEditor() = default;

void {{ProductId}}AudioProcessorEditor::layoutBody (juce::Rectangle<int> area)
{
    const auto outputWidth = juce::jmax (meter.getPreferredWidth(), mixKnob != nullptr ? mixKnob->getPreferredWidth() : 0);
    auto output = area.removeFromRight (outputWidth);
    area.removeFromRight (kGroupGap);
    outputRule = area.removeFromRight (1);
    area.removeFromRight (kGroupGap);

    // CONTROLS
    controlsTitle = area.removeFromTop (14);
    area.removeFromTop (Tokens::space3);
    auto x = area.getX();
    for (auto& k : knobs)
    {
        k->setBounds (x, area.getY(), k->getPreferredWidth(), k->getPreferredHeight());
        x += k->getPreferredWidth() + kKnobGap;
    }
    for (auto& s : segments)
    {
        s->setBounds (x, area.getY() + (Tokens::knobMd - 28) / 2, s->getPreferredWidth(), 28);
        x += s->getPreferredWidth() + kKnobGap;
    }

    // OUTPUT, last on the right: Mix, the meter, the switches.
    outputTitle = output.removeFromTop (14);
    output.removeFromTop (Tokens::space3);
    if (mixKnob != nullptr)
    {
        mixKnob->setBounds (output.removeFromTop (mixKnob->getPreferredHeight()).withSizeKeepingCentre (mixKnob->getPreferredWidth(), mixKnob->getPreferredHeight()));
        output.removeFromTop (Tokens::space5);
    }
    meter.setBounds (output.removeFromTop (LevelMeter::kHeight).withWidth (meter.getPreferredWidth()));
    if (! toggles.empty())
        output.removeFromTop (Tokens::space4);
    for (auto& t : toggles)
        t->setBounds (output.removeFromTop (28).withWidth (t->getPreferredWidth()));
}

void {{ProductId}}AudioProcessorEditor::paint (juce::Graphics& g)
{
    StripEditor::paint (g);
    const auto& t = front();
    type::draw (g, "CONTROLS", controlsTitle.toFloat(), type::section(), t.inkMuted);
    type::draw (g, "OUTPUT", outputTitle.toFloat(), type::section(), t.inkMuted);
    g.setColour (t.line);
    g.fillRect (outputRule);
}

void {{ProductId}}AudioProcessorEditor::advanceBody (double now, float dt)
{
    for (auto& k : knobs)
        k->advance (now, dt);
    if (mixKnob != nullptr)
        mixKnob->advance (now, dt);
    meter.advance (now, dt);
}

void {{ProductId}}AudioProcessorEditor::presetRecalled (double now)
{
    // The knobs glide to the recalled values one after another, left to right.
    auto start = now;
    for (auto& k : knobs)
    {
        k->knob().beginWave (start, false);
        start += 0.04;
    }
    if (mixKnob != nullptr)
        mixKnob->knob().beginWave (start, false);
}

bool {{ProductId}}AudioProcessorEditor::previewDrag (const juce::String& parameterId)
{
    auto* parameter = audioProcessor.parameters().getParameter (parameterId);
    if (parameter == nullptr)
        return false;

    for (auto* k : { mixKnob.get() })
        if (k != nullptr && k->knob().getTitle() == parameter->getName (64))
        {
            k->knob().setDragPreview (true);
            return true;
        }
    for (auto& k : knobs)
        if (k->knob().getTitle() == parameter->getName (64))
        {
            k->knob().setDragPreview (true);
            return true;
        }
    return false;
}
