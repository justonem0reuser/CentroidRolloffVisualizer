#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    centroidVisualizer = std::make_unique<CentroidRolloffPanVisualizer>(p);
    addAndMakeVisible(centroidVisualizer.get());

    attach(rollonSlider, rollonLabel, rollonAttachment, p.rollonStr);
    attach(rolloffSlider, rolloffLabel, rolloffAttachment, p.rolloffStr);
    attach(centroidSmoothingSlider, centroidSmoothingLabel, centroidSmoothingAttachment, p.freqStr);
    attach(balanceSmoothingSlider, balanceSmoothingLabel, balanceSmoothingAttachment, p.balStr);

    setSize (800, 600);
    setResizable(true, false);
    setResizeLimits(800, 600, 4000, 4000);
    centreWithSize(getWidth(), getHeight());
    startTimerHz(timerHz);
}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
}

void PluginEditor::resized()
{
    const int w = getWidth(),
              w2 = w / 2,
              sliderWidth = w2 - 1.5 * horPadding - labelWidth,
              x1 = horPadding + labelWidth,
              x2 = w2 + 0.5 * horPadding + labelWidth,
              y1 = vertPadding,
              y2 = y1 + sliderHeight,
              yv = y2 + sliderHeight;

    centroidSmoothingSlider.setBounds(x1, y1, sliderWidth, sliderHeight);
    balanceSmoothingSlider.setBounds(x1, y2, sliderWidth, sliderHeight);
    rollonSlider.setBounds(x2, y1, sliderWidth, sliderHeight);
    rolloffSlider.setBounds(x2, y2, sliderWidth, sliderHeight);
    
    centroidVisualizer->setBounds(0, yv, getWidth(), getHeight() - yv);
}

void PluginEditor::timerCallback()
{
    if (audioProcessor.nextFFTBlockReady[0])
    {
        audioProcessor.drawNextFrameOfSpectrum();
        audioProcessor.nextFFTBlockReady[0] = audioProcessor.nextFFTBlockReady[1] = false;
        repaint();
    }
}

void PluginEditor::attach(juce::Slider& slider, 
                          juce::Label& label, 
                          std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment, 
                          const juce::String& id)
{
    attachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.apvts, id, slider));
    addAndMakeVisible(&slider);
    addAndMakeVisible(&label);
    auto par = audioProcessor.apvts.getParameter(id);
    label.setText(par->getName(100), juce::NotificationType::dontSendNotification);
    label.attachToComponent(&slider, true);
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
}
