#pragma once

#include <JuceHeader.h>
#include <vector>

/**
 * Generates standardized test signals for IR capture
 * Implements the Can Damonium IR Standard (v1.0)
 */
class TestSignalGenerator
{
public:
    TestSignalGenerator();
    ~TestSignalGenerator();

    // Initialize for given sample rate
    void prepareToPlay (double sampleRate);

    // Generate signals
    void generateLogarithmicChirp (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void generateWhiteNoise (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    // Configuration
    void setChirpRange (float frequencyStart, float frequencyEnd);
    void setChirpDuration (float durationSeconds);
    void setAmplitude (float amplitudeDb);

    // State
    void reset();
    bool isFinished() const { return finished; }
    float getProgress() const { return progress; }

private:
    double sampleRate = 48000.0;
    
    // Chirp parameters (standard: 20-20000 Hz, 10 seconds, -3 dBFS)
    float frequencyStart = 20.0f;
    float frequencyEnd = 20000.0f;
    float chirpDuration = 10.0f;
    float amplitude = 0.707f; // -3 dBFS

    // State
    double phase = 0.0;
    int samplesGenerated = 0;
    int totalSamples = 0;
    bool finished = false;
    float progress = 0.0f;

    // Random number generator for noise
    juce::Random rng;

    // Fade envelope
    void applyFade (juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestSignalGenerator)
};
