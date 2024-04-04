#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CentroidRolloffPanVisualizer.h"

class PluginEditor  : 
    public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    PluginEditor(PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    PluginProcessor& audioProcessor;
    const int timerHz = 15,
              textBoxWidth = 50,
              textBoxHeight = 20,
              labelWidth = 200,
              sliderHeight = 20,
              horPadding = 40,
              vertPadding = 10;

    juce::Slider rollonSlider, 
                 rolloffSlider, 
                 centroidSmoothingSlider, 
                 balanceSmoothingSlider, 
                 fftOrderSlider;

    juce::Label rollonLabel,
                rolloffLabel,
                centroidSmoothingLabel,
                balanceSmoothingLabel,
                fftOrderLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        rollonAttachment,
        rolloffAttachment,
        centroidSmoothingAttachment,
        balanceSmoothingAttachment,
        fftOrderAttachment;

    std::unique_ptr<CentroidRolloffPanVisualizer> centroidVisualizer;

    void attach(juce::Slider& slider,
                juce::Label& label,
                std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment,
                const juce::String& id);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
