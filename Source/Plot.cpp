#include "Plot.h"

Plot::Plot(PluginProcessor& p):
    audioProcessor(p)
{
}

void Plot::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setOpacity(1.0f);
    drawAxes(g);
    drawBalanceRms(g);
    drawCentroidRolloff(g);
}

void Plot::drawAxes(juce::Graphics& g)
{
    float maxX = getWidth() - 1;
    float maxY = getHeight() - 1;
    g.setColour(axesColour);
    for (int i = 0; i <= 10; i++)
    {
        g.setColour(axesColour);
        int x = std::roundf(juce::jmap(i / 10.f, 0.f, 1.f, 0.f, maxX));
        g.drawVerticalLine(x, 0.f, maxY);
    }

    g.setColour(axesColour);
    for (int f = 20; f <= 20000; f += (f < 200 ? 20 : f < 2000 ? 200 : 2000))
    {
        int y = std::roundf(juce::jmap(std::log10f(f), logMinFreq, logMaxFreq, maxY, 0.f));
        g.drawHorizontalLine(y, 0.f, maxX);
    }
}

void Plot::drawCentroidRolloff(juce::Graphics& g)
{
    float width = getWidth();
    float height = getHeight();
    if (height > 0 && width > 0)
    {
        int totalNumInputChannels = audioProcessor.getTotalNumInputChannels();
        float figureWidth = totalNumInputChannels == 1 ? width : 0.5f * width;
        for (int i = 0; i < totalNumInputChannels; i++)
        {
            float centroid = audioProcessor.smoothedCentroid[i],
                  rollon = audioProcessor.smoothedRollon[i],
                  rolloff = audioProcessor.smoothedRolloff[i];
            if (centroid > 0 && rollon > 0 && rolloff > 0)
            {
                float
                    centroidY = juce::jmap(
                        std::log10f(centroid),
                        logMinFreq, logMaxFreq,
                        height, 0.f),
                    rolloffY = juce::jmap(
                        std::log10f(rolloff),
                        logMinFreq, logMaxFreq,
                        height, 0.f),
                    rollonY = juce::jmap(
                        std::log10f(rollon),
                        logMinFreq, logMaxFreq,
                        height, 0.f),
                    rectHeight = rollonY - rolloffY,
                    centroidRelPos = rectHeight == 0.f ? -1.f : (centroidY - rolloffY) / rectHeight,
                    cornerSize = fminf(centroidY - rolloffY, rollonY - centroidY);

                if (rectHeight < 1.f)
                    rectHeight = 1.f;
                if (cornerSize < 0.f)
                    cornerSize = 0.f;
                if (cornerSize > 0.5f * rectHeight)
                    cornerSize = 0.5f * rectHeight;

                juce::Rectangle<float> rect(
                    figureWidth * i, rolloffY, figureWidth, rectHeight);

                juce::ColourGradient gradient = juce::ColourGradient::vertical(
                    rolloffColour, rolloffColour, rect);
                if (centroidRelPos >= 0.f && centroidRelPos <= 1.f)
                    gradient.addColour(centroidRelPos, centroidColour);
                else
                {
                    g.setColour(centroidColour);
                    g.drawHorizontalLine(centroidY, figureWidth * i, figureWidth * (i + 1));
                }

                g.setGradientFill(gradient);
                g.fillRoundedRectangle(rect, cornerSize);
            }
        }
    }
}

void Plot::drawBalanceRms(juce::Graphics& g)
{
    float width = getWidth();
    float height = getHeight();
    if (height > 0 && width > 0)
    {
        float monoRms = audioProcessor.getTotalNumInputChannels() == 1 ?
            audioProcessor.smoothedRms[0] :
            0.5f * (audioProcessor.smoothedRms[0] + audioProcessor.smoothedRms[1]);
        float xBal = juce::jmap(audioProcessor.smoothedBalance,
            -1.0f, 1.0f, 0.f, width);
        float xRms = juce::jmap(monoRms,
            0.f, 1.f, 0.f, 0.5f * width);
        juce::Rectangle<float> balRect(xBal - xRms, 0.f, 2.f * xRms, height);

        juce::ColourGradient balanceGradient = juce::ColourGradient::horizontal(
            edgeBalanceColour, edgeBalanceColour, balRect);
        balanceGradient.addColour(0.5, balanceColour);
        g.setGradientFill(balanceGradient);
        g.fillRect(balRect);
    }
}
