#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FeaturesCalculator.h"

//==============================================================================
PluginProcessor::PluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , forwardFFT(fftOrder),
    window(fftSize, juce::dsp::WindowingFunction<float>::hann),
    balance(0),
    smoothedBalance(0)
{
    rms[0] = rms[1] = 0;
    scopeData[0].resize(scopeSize);
    scopeData[1].resize(scopeSize);
    freqBalance.resize(scopeSize);
    fifo[0].resize(fftSize);
    fifo[1].resize(fftSize);
    fifo[0].resize(fftSize);
    fifo[1].resize(fftSize);
    fftData[0].resize(fftSize * 2);
    fftData[1].resize(fftSize * 2);
}

PluginProcessor::~PluginProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (rollonStr, "Roll-on (%)", 0.05f, 0.50f, 0.05f));
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (rolloffStr, "Roll-off (%)", 0.50f, 0.95f, 0.85f));
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (freqStr, "Frequency smoothing coefficient", 0.0f, 0.99f, 0.33f));
    layout.add(std::make_unique<juce::AudioParameterFloat>
        (balStr, "Balance smoothing coefficient", 0.0f, 0.99f, 0.33f));
    return layout;
}

const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
}

const juce::String PluginProcessor::getProgramName (int index)
{
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void PluginProcessor::drawNextFrameOfSpectrum()
{
    double rate = getSampleRate();
    auto balPar = apvts.getParameter(balStr),
        freqPar = apvts.getParameter(freqStr),
        rollonPar = apvts.getParameter(rollonStr),
        rolloffPar = apvts.getParameter(rolloffStr);
    float balCoeff = balPar->convertFrom0to1(balPar->getValue()),
        freqCoeff = freqPar->convertFrom0to1(freqPar->getValue()),
        rollonFrac = rollonPar->convertFrom0to1(rollonPar->getValue()),
        rolloffFrac = rolloffPar->convertFrom0to1(rolloffPar->getValue());

    int totalNumInputChannels = getTotalNumInputChannels();
    for (int i = 0; i < totalNumInputChannels; i++)
    {
        rms[i] = FeaturesCalculator::rms(fftData[i]);
        FeaturesCalculator::smooth(rms[i], balCoeff, smoothedRms[i]);

        window.multiplyWithWindowingTable(fftData[i].data(), fftSize);
        forwardFFT.performFrequencyOnlyForwardTransform(fftData[i].data());

        FeaturesCalculator::centroidRolloff(
            fftData[i], rate, rollonFrac, rolloffFrac, centroid[i], rollon[i], rolloff[i]);
        FeaturesCalculator::smooth(centroid[i], freqCoeff, smoothedCentroid[i]);
        FeaturesCalculator::smooth(rolloff[i], freqCoeff, smoothedRolloff[i]);
        FeaturesCalculator::smooth(rollon[i], freqCoeff, smoothedRollon[i]);
    }

    if (totalNumInputChannels == 1)
        smoothedBalance = balance = 0.0f;
    else
    {
        balance = FeaturesCalculator::balance(rms[0], rms[1]);
        FeaturesCalculator::smooth(balance, balCoeff, smoothedBalance);
    }
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    fifoIndex[0] = fifoIndex[1] = 0;
    smoothedBalance = balance = 0.0f;
    for (int i = 0; i < 2; i++)
        smoothedRms[i] = smoothedCentroid[i] = smoothedRollon[i] = smoothedRolloff[i] = 
            rms[i] = centroid[i] = rollon[i] = rolloff[i] = 0.0f;
}

void PluginProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    int totalNumInputChannels  = getTotalNumInputChannels();
    int totalNumOutputChannels = getTotalNumOutputChannels();
    int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; channel++)
    {
        auto* channelData = buffer.getReadPointer(channel);
            for (auto i = 0; i < numSamples; i++)
                //if (channelData[i] >= -1.f && channelData[i] <= 1.f) // TODO: ?
                    pushNextSampleIntoFifo(channel, channelData[i]);
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; 
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void PluginProcessor::pushNextSampleIntoFifo(int channel, float sample) noexcept
{
    // if the fifo contains enough data, set a flag to say
    // that the next frame should now be rendered..
    if (fifoIndex[channel] == fftSize)
    {
        if (!nextFFTBlockReady[channel])
        {
            std::fill(fftData[channel].begin(), fftData[channel].end(), 0.0f);
            std::copy(fifo[channel].begin(), fifo[channel].end(), fftData[channel].begin());
            nextFFTBlockReady[channel] = true;
        }

        fifoIndex[channel] = 0;
    }
    fifo[channel][fifoIndex[channel]++] = sample;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
