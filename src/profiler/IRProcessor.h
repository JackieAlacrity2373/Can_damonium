#pragma once

#include <JuceHeader.h>

/**
 * Post-processes recorded IRs to meet standard specifications
 * - Validates format (48 kHz, 24-bit)
 * - Trims decay tail
 * - Normalizes level
 * - Generates metadata
 */
class IRProcessor
{
public:
    IRProcessor();
    ~IRProcessor();

    // Processing pipeline
    bool processRecording (const juce::File& inputFile, const juce::File& outputFile);

    // Validation
    bool validateIR (const juce::File& irFile);

    // Analysis
    float analyzeDecayEnvelope (const juce::AudioBuffer<float>& buffer);
    int findTailEndSample (const juce::AudioBuffer<float>& buffer, float thresholdDb = -80.0f);

    // Utilities
    void normalizeBuffer (juce::AudioBuffer<float>& buffer, float targetDb = -3.0f);
    void trimBuffer (juce::AudioBuffer<float>& buffer, int endSample);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRProcessor)
};
