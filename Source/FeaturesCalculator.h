#pragma once
#include <JuceHeader.h>

static class FeaturesCalculator
{
public:
    static float rms(std::vector<float>& samples);

    static void centroidRolloff(
        std::vector<float>& fft,
        double rate,
        float rollonFrac,
        float rolloffFrac,
        float& centroid,
        float& rollon,
        float& rolloff);

    static float balance(float leftGain, float rightGain);

    static void smooth(float real, float coeff, float& smoothed);

private:
    static float search(std::vector<float>& data, float percent);
};