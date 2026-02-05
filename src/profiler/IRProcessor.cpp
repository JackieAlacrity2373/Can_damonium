#include "IRProcessor.h"
#include <cmath>

IRProcessor::IRProcessor()
{
}

IRProcessor::~IRProcessor()
{
}

bool IRProcessor::processRecording (const juce::File& inputFile, const juce::File& outputFile)
{
    // TODO: Load, process, and save IR
    return false;
}

bool IRProcessor::validateIR (const juce::File& irFile)
{
    // TODO: Validate IR against standard spec
    return false;
}

float IRProcessor::analyzeDecayEnvelope (const juce::AudioBuffer<float>& buffer)
{
    // TODO: Analyze envelope decay characteristics
    return 0.0f;
}

int IRProcessor::findTailEndSample (const juce::AudioBuffer<float>& buffer, float thresholdDb)
{
    auto* data = buffer.getReadPointer (0);
    int numSamples = buffer.getNumSamples();

    float thresholdLinear = std::pow (10.0f, thresholdDb / 20.0f);

    // Search backwards from end for last sample above threshold
    for (int i = numSamples - 1; i >= 0; --i)
    {
        if (std::abs (data[i]) > thresholdLinear)
            return i;
    }

    return 0;
}

void IRProcessor::normalizeBuffer (juce::AudioBuffer<float>& buffer, float targetDb)
{
    auto* data = buffer.getWritePointer (0);
    int numSamples = buffer.getNumSamples();

    float maxLevel = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        maxLevel = std::max (maxLevel, std::abs (data[i]));

    if (maxLevel > 0.0f)
    {
        float targetLinear = std::pow (10.0f, targetDb / 20.0f);
        float gain = targetLinear / maxLevel;

        for (int i = 0; i < numSamples; ++i)
            data[i] *= gain;
    }
}

void IRProcessor::trimBuffer (juce::AudioBuffer<float>& buffer, int endSample)
{
    if (endSample < buffer.getNumSamples())
    {
        buffer.setSize (buffer.getNumChannels(), endSample);
    }
}
