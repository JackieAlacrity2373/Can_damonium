#pragma once

#include <JuceHeader.h>

//==============================================================================
// Simple test tone generator for debugging
class TestToneGenerator
{
public:
    TestToneGenerator() : phase(0), frequency(440.0), sampleRate(48000.0) {}
    
    void setSampleRate(double sr) { sampleRate = sr; }
    void setFrequency(double freq) { frequency = freq; }
    
    void generateBlock(juce::AudioBuffer<float>& buffer, int numSamples)
    {
        auto* left = buffer.getWritePointer(0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
        
        double phaseDelta = frequency / sampleRate;
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Simple sine wave at very low level (so convolution effect is audible)
            float sample = std::sin(2.0f * juce::MathConstants<float>::pi * (float)phase) * 0.1f;
            
            left[i] = sample;
            if (right) right[i] = sample;
            
            phase += phaseDelta;
            if (phase >= 1.0) phase -= 1.0;
        }
    }
    
    void reset() { phase = 0; }

private:
    double phase;
    double frequency;
    double sampleRate;
};
