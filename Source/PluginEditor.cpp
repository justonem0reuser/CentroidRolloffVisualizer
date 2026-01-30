#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor (&p), 
    audioProcessor (p),
    forwardFFT(p.fftOrder),
    window(p.fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    centroidVisualizer = std::make_unique<CentroidRolloffPanVisualizer>(p);
    addAndMakeVisible(centroidVisualizer.get());

    attach(rollonSlider, rollonLabel, rollonAttachment, p.rollonStr);
    attach(rolloffSlider, rolloffLabel, rolloffAttachment, p.rolloffStr);
    attach(centroidSmoothingSlider, centroidSmoothingLabel, centroidSmoothingAttachment, p.freqStr);
    attach(balanceSmoothingSlider, balanceSmoothingLabel, balanceSmoothingAttachment, p.balStr);

    setSize (800, 600);
    setResizable(true, false);
    setResizeLimits(600, 300, 4000, 4000);
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
    if (!audioProcessor.nextFFTBlockReady.load(std::memory_order_acquire))
        return;

    int start1, size1, start2, size2;
    audioProcessor.abstractFifo.prepareToRead(audioProcessor.fftSize, start1, size1, start2, size2);

    if (size1 + size2 < audioProcessor.fftSize)
        return; // just in case

    int channelNumber = audioProcessor.getNumInputChannels(); // always > 0 here

    for (int i = 0; i < channelNumber; i++)
    {
        std::fill(fftData[i].begin() + audioProcessor.fftSize, 
            fftData[i].end(), 
            0.0f);

        std::copy(audioProcessor.fifo[i].begin() + start1,
            audioProcessor.fifo[i].begin() + start1 + size1,
            fftData[i].begin());

        if (size2 > 0)
            std::copy(audioProcessor.fifo[i].begin() + start2,
                audioProcessor.fifo[i].begin() + start2 + size2,
                fftData[i].begin() + size1);
    }

    drawNextFrameOfSpectrum();

    audioProcessor.abstractFifo.finishedRead(audioProcessor.fftSize);
    audioProcessor.nextFFTBlockReady.store(false, std::memory_order_release);

    repaint();
}

void PluginEditor::drawNextFrameOfSpectrum()
{
    float balCoeff = audioProcessor.apvts.getRawParameterValue(audioProcessor.balStr)->load(std::memory_order_relaxed);
    float freqCoeff = audioProcessor.apvts.getRawParameterValue(audioProcessor.freqStr)->load(std::memory_order_relaxed);
    float rollonFrac = audioProcessor.apvts.getRawParameterValue(audioProcessor.rollonStr)->load(std::memory_order_relaxed);
    float rolloffFrac = audioProcessor.apvts.getRawParameterValue(audioProcessor.rolloffStr)->load(std::memory_order_relaxed);

    int totalNumInputChannels = audioProcessor.getTotalNumInputChannels();
    double rate = audioProcessor.getSampleRate();

    std::array<float, 2> rms{};
    for (int i = 0; i < totalNumInputChannels; i++)
    {
        rms[i] = FeaturesCalculator::rms(fftData[i]);
        FeaturesCalculator::smooth(rms[i], balCoeff, audioProcessor.smoothedRms[i]);

        window.multiplyWithWindowingTable(fftData[i].data(), audioProcessor.fftSize);
        forwardFFT.performFrequencyOnlyForwardTransform(fftData[i].data());

        float centroid, rollon, rolloff;
        FeaturesCalculator::centroidRolloff(
            fftData[i], rate, rollonFrac, rolloffFrac, centroid, rollon, rolloff);
        FeaturesCalculator::smooth(centroid, freqCoeff, audioProcessor.smoothedCentroid[i]);
        FeaturesCalculator::smooth(rolloff, freqCoeff, audioProcessor.smoothedRolloff[i]);
        FeaturesCalculator::smooth(rollon, freqCoeff, audioProcessor.smoothedRollon[i]);
    }

    if (totalNumInputChannels == 1)
        audioProcessor.smoothedBalance = 0.f;
    else
    {
        float balance = FeaturesCalculator::balance(rms[0], rms[1]);
        FeaturesCalculator::smooth(balance, balCoeff, audioProcessor.smoothedBalance);
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
