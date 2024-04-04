#pragma once

#include <JuceHeader.h>

class PluginProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    const juce::String rollonStr = "Rollon",
                       rolloffStr = "Rolloff",
                       freqStr = "FreqCoeff",
                       balStr = "BalCoeff";
    const float minDb = -100.f,
                maxDb = 0.f;
    static const int fftOrder = 11,
                     fftSize = 1 << fftOrder,
                     scopeSize = 512;

    std::vector<float> scopeData[2];
    float rms[2];
    float centroid[2];
    float rollon[2];
    float rolloff[2];
    float balance;
    float smoothedRms[2];
    float smoothedCentroid[2];
    float smoothedRollon[2];
    float smoothedRolloff[2];
    float smoothedBalance;
    std::vector<float> freqBalance;
    bool nextFFTBlockReady[2] { false, false };

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    PluginProcessor();
    ~PluginProcessor() override;

    void drawNextFrameOfSpectrum();
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::dsp::FFT forwardFFT;                      
    juce::dsp::WindowingFunction<float> window;     
    int fifoIndex[2] { 0, 0 };
    std::vector<float> fifo[2];
    std::vector<float> fftData[2];
    //float fifo[fftSize];
    //float fftData[2 * fftSize];
    void pushNextSampleIntoFifo(int channel, float sample) noexcept;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
