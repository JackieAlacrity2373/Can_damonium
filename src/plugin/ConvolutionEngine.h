#pragma once

#include <JuceHeader.h>

/**
 * Manages impulse response files and convolution operations
 */
class ConvolutionEngine
{
public:
    ConvolutionEngine();
    ~ConvolutionEngine();

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void processBlock (juce::AudioBuffer<float>& buffer);

    bool loadImpulseResponse (const juce::File& irFile);
    bool loadImpulseResponseFromMemory (const void* data, size_t size);
    bool isIrLoaded() const noexcept { return irLoaded; }
    
    void setBypass (bool shouldBypass) noexcept { bypass.store(shouldBypass); }
    bool isBypassed() const noexcept { return bypass.load(); }

    void setIrResampleEnabled (bool enabled) noexcept { resampleIrToDevice.store(enabled); }
    bool isIrResampleEnabled() const noexcept { return resampleIrToDevice.load(); }
    
    // Deferred IR loading: Store IR to load on next prepareToPlay or processBlock
    void setDeferredIRLoad (const juce::File& irFile) noexcept { deferredIRFile = irFile; }
    
    // Signal that we need to reset the convolver's buffers after IR load
    void signalIRChange() noexcept { needsReset.store(true); }

private:
    juce::dsp::Convolution convolver;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    double lastPreparedSampleRate = 0.0;
    int lastPreparedBlockSize = 0;
    std::atomic<bool> irLoaded { false }; // Indicates if an impulse response is loaded
    std::atomic<bool> bypass { false }; // Bypass convolution processing
    std::atomic<bool> isPrepared { false }; // Flag: convolver is already prepared
    std::atomic<bool> needsReset { false }; // Flag: reset convolver buffers on next processBlock
    std::atomic<bool> resampleIrToDevice { true }; // Resample IR to device sample rate on load
    juce::String lastLoadedIRPath; // Track which IR is loaded to prevent reloading same file
    juce::File deferredIRFile; // IR to load after prepareToPlay is called

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvolutionEngine)
};
