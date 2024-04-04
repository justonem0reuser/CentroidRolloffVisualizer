#include "FeaturesCalculator.h"

float FeaturesCalculator::rms(std::vector<float>& samples)
{
    const int size = samples.size();
    float res = 0.f;
    for (int i = 0; i < size; i++)
        res += samples[i] * samples[i];
    return std::sqrt(res / size);
}

void FeaturesCalculator::centroidRolloff(std::vector<float>& fft, double rate, float rollonFrac, float rolloffFrac, float& centroid, float& rollon, float& rolloff)
{
    const int size = fft.size();
    if (size % 4 > 0)
        throw std::exception("Wrong fft size");
    const int n = size / 4 + 1;
    const float freqCoeff = rate * 2 / size;
    float sum = 0.f;
    float numerator = 0.f;
    std::vector<float> e(n);

    for (int i = 0; i < n; i++)
    {
        numerator += fft[i] * i;
        sum += fft[i];
        e[i] = sum;
    }
    if (sum == 0 || freqCoeff == 0)
    {
        centroid = rollon = rolloff = 0;
        return;
    }
    centroid = freqCoeff * numerator / sum;
    rollon = search(e, rollonFrac) * freqCoeff;
    rolloff = search(e, rolloffFrac) * freqCoeff;
}

float FeaturesCalculator::balance(float leftGain, float rightGain)
{
    if (rightGain == 0)
    {
        if (leftGain == 0)
            return 0.f;
        return -1.f;
    }
    float k = leftGain / rightGain;
    return k <= 1.f ?
        1.f - k :
        -1.f + 1.f / k;
}

void FeaturesCalculator::smooth(float real, float coeff, float& smoothed)
{
    smoothed = coeff * (smoothed - real) + real;
}

float FeaturesCalculator::search(std::vector<float>& data, float percent)
{     // TODO: test!!!
    int n = data.size(),
        left = 0,
        right = n - 1;
    float val = percent * data[n - 1];
    if (n < 2 || val <= data[0])
        return 0.0f;
    if (val >= data[right])
        return (float)right;

    int i = (int)(percent * (n - 1));   
    while (left < right - 1)
    {
        if (data[i] <= val)
            left = i;
        else
            right = i;
        i = (left + right) / 2;
    }

    // percent < 1.0 => i < n - 1;   data[i + 1] > data[i]
    return (val - data[i]) / (data[i + 1] - data[i]) + i;
}
