#include "TestSignalGenerator.h"
#include <cmath>

TestSignalGenerator::TestSignalGenerator()
    : rng (juce::Random::getSystemRandom())
{
}

TestSignalGenerator::~TestSignalGenerator()
{
}

void TestSignalGenerator::prepareToPlay (double sr)
{
    sampleRate = sr;
    totalSamples = static_cast<int> (sampleRate * chirpDuration);
    reset();
}

void TestSignalGenerator::generateLogarithmicChirp (juce::AudioBuffer<float>& buffer, 
                                                    int startSample, int numSamples)
{
    auto* channelData = buffer.getWritePointer (0, startSample);
    
    const float k = (frequencyEnd - frequencyStart) / chirpDuration;
    const float phase0 = frequencyStart;

    for (int i = 0; i < numSamples; ++i)
    {
        if (samplesGenerated >= totalSamples)
        {
            finished = true;
            channelData[i] = 0.0f;
            continue;
        }

        // Time in seconds
        float t = samplesGenerated / static_cast<float> (sampleRate);

        // Logarithmic chirp: f(t) = f0 * (f1/f0)^(t/T)
        float freq = frequencyStart * std::pow (frequencyEnd / frequencyStart, t / chirpDuration);

        // Phase accumulation: phi(t) = 2*pi * integral(f dt)
        phase += 2.0 * juce::MathConstants<double>::pi * freq / sampleRate;

        // Generate sine
        float sample = amplitude * std::sin (static_cast<float> (phase));

        // Apply fade envelope
        float fade = 1.0f;
        const int fadeLength = static_cast<int> (0.05 * sampleRate); // 50ms fade

        if (i < fadeLength)
            fade = i / static_cast<float> (fadeLength);
        else if (i > numSamples - fadeLength)
            fade = (numSamples - i) / static_cast<float> (fadeLength);

        channelData[i] = sample * fade;
        samplesGenerated++;
    }

    progress = static_cast<float> (samplesGenerated) / totalSamples;
}

void TestSignalGenerator::generateWhiteNoise (juce::AudioBuffer<float>& buffer, 
                                              int startSample, int numSamples)
{
    auto* channelData = buffer.getWritePointer (0, startSample);

    for (int i = 0; i < numSamples; ++i)
    {
        // Generate white noise in range [-1, 1]
        float noise = rng.nextFloat() * 2.0f - 1.0f;
        channelData[i] = noise * amplitude;
        samplesGenerated++;
    }

    progress = static_cast<float> (samplesGenerated) / totalSamples;
}

void TestSignalGenerator::setChirpRange (float frequencyStart_, float frequencyEnd_)
{
    frequencyStart = frequencyStart_;
    frequencyEnd = frequencyEnd_;
}

void TestSignalGenerator::setChirpDuration (float durationSeconds)
{
    chirpDuration = durationSeconds;
}

void TestSignalGenerator::setAmplitude (float amplitudeDb)
{
    amplitude = std::pow (10.0f, amplitudeDb / 20.0f);
}

void TestSignalGenerator::reset()
{
    samplesGenerated = 0;
    phase = 0.0;
    finished = false;
    progress = 0.0f;
    totalSamples = static_cast<int> (sampleRate * chirpDuration);
}

void TestSignalGenerator::applyFade (juce::AudioBuffer<float>& buffer, 
                                     int startSample, int numSamples)
{
    auto* channelData = buffer.getWritePointer (0, startSample);
    const int fadeLength = static_cast<int> (0.05 * sampleRate); // 50ms

    for (int i = 0; i < numSamples; ++i)
    {
        float fade = 1.0f;

        if (i < fadeLength)
            fade = i / static_cast<float> (fadeLength);
        else if (i > numSamples - fadeLength)
            fade = (numSamples - i) / static_cast<float> (fadeLength);

        channelData[i] *= fade;
    }
}
