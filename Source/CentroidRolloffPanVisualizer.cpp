#include <JuceHeader.h>
#include "CentroidRolloffPanVisualizer.h"

//==============================================================================
CentroidRolloffPanVisualizer::CentroidRolloffPanVisualizer(PluginProcessor& p):
    audioProcessor(p)
{
    plot = std::make_unique<Plot>(p);
    addAndMakeVisible(plot.get());
}

void CentroidRolloffPanVisualizer::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setOpacity(1.0f);
    drawAxesText(g);
}

void CentroidRolloffPanVisualizer::resized()
{
    plot->setBounds(plotMargin, 
                    plotMargin, 
                    getWidth() - 2 * plotMargin, 
                    getHeight() - 2 * plotMargin);
}

void CentroidRolloffPanVisualizer::drawAxesText(juce::Graphics& g)
{
    float height = getHeight();
    float width = getWidth();
    g.setColour(textColour);
    auto font = g.getCurrentFont();
    float yBias = 0.5f * font.getHeight();
    for (int i = 0; i <= 10; i++)
    {
        int x = std::roundf(juce::jmap(i / 10.f, 0.f, 1.f, plotMargin, width - plotMargin));
        float xBias = 0.5f * font.getStringWidthFloat(xAxisValues[i]);
        g.setColour(textColour);
        g.drawSingleLineText(xAxisValues[i],
            x - xBias,
            height - plotMargin + 2.f * yBias + 2.f);
    }

    g.setColour(textColour);
    for (int i = 0; i <= 3; i++)
        g.drawSingleLineText(yAxisValues[i], 2, 
            juce::jmap((float)i, 0.f, 3.f, height - plotMargin, plotMargin) + yBias);

    g.drawSingleLineText(xAxisName, 0.5f * width, plotMargin - 5,
        juce::Justification::horizontallyCentred);

    juce::GlyphArrangement ga;
    ga.addLineOfText(g.getCurrentFont(), yAxisName, 0, 0);
    juce::Path p;
    ga.createPath(p);
    auto pathBounds = p.getBounds();
    p.applyTransform(juce::AffineTransform()
        .rotated(juce::degreesToRadians(90.f),
            pathBounds.getCentreX(),
            pathBounds.getCentreY())
        .translated(getWidth() - plotMargin + 2.f * yBias + 2 - pathBounds.getCentreX(),
                    0.5f * getHeight()  - pathBounds.getCentreY())
    );
    g.fillPath(p);
}