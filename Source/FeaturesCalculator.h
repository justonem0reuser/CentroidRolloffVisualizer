#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class FeaturesCalculator
{
public:
    static float rms(std::array<float, 2 * PluginProcessor::fftSize>& samples);

    static void centroidRolloff(
        std::array<float, 2 * PluginProcessor::fftSize>& fft,
        double rate,
        float rollonFrac,
        float rolloffFrac,
        float& centroid,
        float& rollon,
        float& rolloff);

    static float balance(float leftGain, float rightGain);

    static void smooth(float real, float coeff, float& smoothed);

private:
    static float search(std::array<float, PluginProcessor::fftSize / 2 + 1>& data, float percent);
};