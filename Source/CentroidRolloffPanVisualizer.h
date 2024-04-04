#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Plot.h"

//==============================================================================
/*
*/
class CentroidRolloffPanVisualizer  : public juce::Component
{
public:
    CentroidRolloffPanVisualizer(PluginProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    std::unique_ptr<Plot> plot;
    const juce::Colour textColour = juce::Colours::lightgrey;
    const float plotMargin = 42.f;
    juce::Path yAxisNamePath;
    const juce::String xAxisName = "Balance", yAxisName = "Frequency, Hz";
    const std::array<juce::String, 11> xAxisValues
    {
        "0.0", "0.1", "0.2", "0.3", "0.4", "0.5", 
        "0.6", "0.7", "0.8", "0.9", "1.0" 
    };
    const std::array<juce::String, 4> yAxisValues { "20", "200", "2000", "20000" };

    PluginProcessor& audioProcessor;

    void drawAxesText(juce::Graphics& g);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CentroidRolloffPanVisualizer)
};
