#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class Plot : public juce::Component
{
public:
    Plot(PluginProcessor& p);

    void paint(juce::Graphics&) override;
private:
    const juce::Colour
        balanceColour = juce::Colours::aqua,
        edgeBalanceColour = balanceColour.withAlpha(0.f),
        centroidColour = juce::Colours::greenyellow,
        rolloffColour = centroidColour.withAlpha(0.15f),
        axesColour = juce::Colours::darkgrey;
    const float
        minFreq = 20.f,
        maxFreq = 20000.f;
    const float
        logMinFreq = std::log10f(minFreq),
        logMaxFreq = std::log10f(maxFreq),
        logMaxDivMinFreq = std::log10f(maxFreq / minFreq);

    PluginProcessor& audioProcessor;

    void drawAxes(juce::Graphics& g);
    void drawCentroidRolloff(juce::Graphics& g);
    void drawBalanceRms(juce::Graphics& g);


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Plot)
};