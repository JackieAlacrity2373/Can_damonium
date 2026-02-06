#pragma once

#include <JuceHeader.h>

/**
 * IR buffer with sample rate info - passed between threads
 */
struct IRBufferWithSampleRate
{
    IRBufferWithSampleRate() = default;
    
    IRBufferWithSampleRate(juce::AudioBuffer<float>&& buf, double sr, int channels)
        : buffer(std::move(buf)), sampleRate(sr), numChannels(channels) {}
    
    juce::AudioBuffer<float> buffer;
    double sampleRate = 0.0;
    int numChannels = 0;
};

/**
 * Thread-safe IR buffer transfer using spinlock
 * UI thread queues new IR, audio thread loads it (non-blocking)
 */
class IRBufferTransfer
{
public:
    void set(IRBufferWithSampleRate&& buf)
    {
        const juce::SpinLock::ScopedLockType lock(mutex);
        buffer = std::move(buf);
        newBuffer = true;
    }
    
    // Call fn with new buffer if available (non-blocking try-lock)
    template <typename Fn>
    void get(Fn&& fn)
    {
        const juce::SpinLock::ScopedTryLockType lock(mutex);
        
        if (lock.isLocked() && newBuffer)
        {
            fn(buffer);
            newBuffer = false;
        }
    }
    
private:
    IRBufferWithSampleRate buffer;
    bool newBuffer = false;
    juce::SpinLock mutex;
};

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
    
    // Deferred IR loading: Store IR to load on next prepareToPlay or processBlock
    void setDeferredIRLoad (const juce::File& irFile) noexcept { deferredIRFile = irFile; }
    
    // Queue an IR buffer for loading on audio thread (via BufferTransfer)
    void queueIRBuffer(IRBufferWithSampleRate&& buf) noexcept { bufferTransfer.set(std::move(buf)); }

private:
    juce::dsp::Convolution convolver;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    double lastPreparedSampleRate = 0.0;
    int lastPreparedBlockSize = 0;
    std::atomic<bool> irLoaded { false }; // Indicates if an impulse response is loaded
    std::atomic<bool> bypass { false }; // Bypass convolution processing
    std::atomic<bool> isPrepared { false }; // Flag: convolver is already prepared
    juce::String lastLoadedIRPath; // Track which IR is loaded to prevent reloading same file
    juce::File deferredIRFile; // IR to load after prepareToPlay is called
    
    // Thread-safe IR buffer queue (UI thread adds, audio thread consumes)
    IRBufferTransfer bufferTransfer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvolutionEngine)
};
