#include "PluginProcessor.h"
#include "PluginEditor.h"

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
{
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

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    smoothedBalance = 0.f;
    for (int i = 0; i < 2; i++)
        smoothedRms[i] = smoothedCentroid[i] = smoothedRollon[i] = smoothedRolloff[i] = 0.f;
    abstractFifo.reset();
    nextFFTBlockReady.store(false, std::memory_order_release);
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
    int totalNumInputChannels  = std::min(getTotalNumInputChannels(), 2);
    if (totalNumInputChannels == 0)
        return;

    int numSamples = buffer.getNumSamples();
    auto* channelData0 = buffer.getReadPointer(0);
    auto* channelData1 = buffer.getReadPointer(totalNumInputChannels == 2 ? 1 : 0);
    auto& fifo0 = fifo[0];
    auto& fifo1 = fifo[1];

    for (auto i = 0; i < numSamples; i++)
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite(1, start1, size1, start2, size2);

        if (size1 > 0) // size2 is always 0 as only 1 sample is requested to be written
        {
            fifo0[start1] = channelData0[i];
            fifo1[start1] = channelData1[i];
            abstractFifo.finishedWrite(size1);
            if (abstractFifo.getNumReady() == fftSize)
                nextFFTBlockReady.store(true, std::memory_order_release);
        }
        //else
        //    abstractFifo.finishedWrite(size1);
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
    juce::MemoryOutputStream mos(destData, true);
    apvts.copyState().writeToStream(mos);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
        apvts.replaceState(tree);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
