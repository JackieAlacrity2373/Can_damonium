#include "ConvolutionEngine.h"

ConvolutionEngine::ConvolutionEngine()
{
    DBG("=== ConvolutionEngine CONSTRUCTOR ===");
}

ConvolutionEngine::~ConvolutionEngine()
{
    DBG("=== ConvolutionEngine DESTRUCTOR ===");
}

void ConvolutionEngine::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::Logger::writeToLog("=== ConvolutionEngine::prepareToPlay START ===");
    juce::Logger::writeToLog("  sampleRate: " + juce::String(sampleRate) + ", blockSize: " + juce::String(samplesPerBlock));
    
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    // Prepare on first call or when device settings change
    const bool needsPrepare = !isPrepared.load()
        || lastPreparedSampleRate != sampleRate
        || lastPreparedBlockSize != samplesPerBlock;

    if (needsPrepare)
    {
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = samplesPerBlock;
        spec.numChannels = 2;
        convolver.prepare(spec);
        isPrepared.store(true);
        lastPreparedSampleRate = sampleRate;
        lastPreparedBlockSize = samplesPerBlock;
        juce::Logger::writeToLog("  Convolver prepared (" + juce::String(sampleRate) + " Hz, block " + juce::String(samplesPerBlock) + ")");
        
        // If an IR was deferred (UI loaded IR before prepareToPlay), load it now
        if (deferredIRFile.existsAsFile())
        {
            juce::Logger::writeToLog("  Loading deferred IR: " + deferredIRFile.getFileName());
            loadImpulseResponse(deferredIRFile);
            deferredIRFile = juce::File(); // Clear the deferred file
        }
        else if (irLoaded.load() && lastLoadedIRPath.isNotEmpty())
        {
            juce::Logger::writeToLog("  Reloading IR after prepareToPlay change: " + lastLoadedIRPath);
            loadImpulseResponse(juce::File(lastLoadedIRPath));
        }
    }
    else
    {
        juce::Logger::writeToLog("  Convolver already prepared - skipping re-prepare");
    }
    
    juce::Logger::writeToLog("=== ConvolutionEngine::prepareToPlay END ===" );
}

void ConvolutionEngine::processBlock (juce::AudioBuffer<float>& buffer)
{
    static int processCallCount = 0;
    processCallCount++;
    
    // CHECK FOR NEW IR EVERY BLOCK (audio thread, non-blocking via try-lock)
    // This is the key difference from the previous implementation
    bufferTransfer.get([this](IRBufferWithSampleRate& irBuf)
    {
        juce::Logger::writeToLog(">>> Processing new IR buffer on audio thread (block #" + juce::String(processCallCount) + ")");
        convolver.loadImpulseResponse(std::move(irBuf.buffer),
                                       irBuf.sampleRate,
                                       irBuf.numChannels == 2 ? juce::dsp::Convolution::Stereo::yes : juce::dsp::Convolution::Stereo::no,
                                       juce::dsp::Convolution::Trim::no,
                                       juce::dsp::Convolution::Normalise::yes);
        irLoaded.store(true);
        juce::Logger::writeToLog(">>> IR loaded successfully on audio thread");
    });
    
    // RULE 1: If bypassed OR no IR loaded -> pass audio through unchanged
    // RULE 2: If IR loaded AND not bypassed -> apply convolution
    
    bool shouldBypass = bypass.load() || !irLoaded.load();
    
    if (shouldBypass)
    {
        // Bypass mode or no IR - audio passes through unchanged
        static int bypassLogCount = 0;
        if (bypassLogCount++ < 5)
        {
            float level = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                level = juce::jmax(level, buffer.getRMSLevel(ch, 0, buffer.getNumSamples()));
            
            juce::String reason = bypass ? "BYPASS=ON" : "NO_IR";
            juce::Logger::writeToLog("PASSTHROUGH #" + juce::String(bypassLogCount) + " (" + reason + 
                                     ") - channels=" + juce::String(buffer.getNumChannels()) + 
                                     " level=" + juce::String(level, 4));
        }
        // Buffer already contains input audio - just return
        return;
    }
    
    // IR is loaded AND bypass is OFF - apply convolution
    static int convolveLogCount = 0;
    bool shouldLog = (convolveLogCount < 10);
    
    if (shouldLog)
    {
        float inLevel = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            inLevel = juce::jmax(inLevel, buffer.getRMSLevel(ch, 0, buffer.getNumSamples()));
        juce::Logger::writeToLog(">>> CONVOLVE #" + juce::String(convolveLogCount + 1) + " - INPUT level=" + 
                                 juce::String(inLevel, 4) + " channels=" + juce::String(buffer.getNumChannels()) + 
                                 " samples=" + juce::String(buffer.getNumSamples()));
    }
    
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    
    try
    {
        convolver.process (context);
        convolveLogCount++;
        
        if (shouldLog)
        {
            float outLevel = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                outLevel = juce::jmax(outLevel, buffer.getRMSLevel(ch, 0, buffer.getNumSamples()));
            juce::Logger::writeToLog(">>> CONVOLVE #" + juce::String(convolveLogCount) + " - OUTPUT level=" + juce::String(outLevel, 4));
        }
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("ERROR in convolver.process(): " + juce::String(e.what()));
        // On error, clear the buffer to prevent glitches
        buffer.clear();
    }
}

bool ConvolutionEngine::loadImpulseResponse (const juce::File& irFile)
{
    juce::Logger::writeToLog("=== ConvolutionEngine::loadImpulseResponse START (file loading, will queue for audio thread) ===");
    juce::Logger::writeToLog("  File: " + irFile.getFullPathName());

    if (irLoaded.load() && lastLoadedIRPath == irFile.getFullPathName())
    {
        juce::Logger::writeToLog("  IR already loaded - skipping reload");
        return true;
    }
    
    if (!irFile.existsAsFile())
    {
        juce::Logger::writeToLog("  ERROR: IR file not found!");
        irLoaded = false;
        return false;
    }
    
    juce::Logger::writeToLog("  File exists - reading audio file...");
    
    try
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(irFile));
        if (reader == nullptr)
        {
            juce::Logger::writeToLog("  ERROR: Cannot read audio file!");
            irLoaded = false;
            return false;
        }

        juce::Logger::writeToLog("  File info: " + juce::String((int)reader->lengthInSamples) +
                                 " samples, " + juce::String(reader->numChannels) +
                                 " channels, " + juce::String((int)reader->sampleRate) + " Hz");

        juce::AudioBuffer<float> irBuffer((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(irBuffer.getArrayOfWritePointers(), irBuffer.getNumChannels(), 0, irBuffer.getNumSamples());

        double irSampleRate = reader->sampleRate;
        int irChannels = (int)reader->numChannels;
        
        // Check for sample rate mismatch and warn user
        if (currentSampleRate > 0.0 && std::abs(irSampleRate - currentSampleRate) > 0.1)
        {
            juce::Logger::writeToLog("  WARNING: IR sample rate (" + juce::String((int)irSampleRate) + 
                                     " Hz) != device sample rate (" + juce::String((int)currentSampleRate) +
                                     " Hz)");
            juce::Logger::writeToLog("  For best results, record IRs at your device's native sample rate.");
            juce::Logger::writeToLog("  JUCE will attempt internal resampling...");
        }

        juce::Logger::writeToLog("  File loaded - queueing to audio thread via BufferTransfer...");
        
        // Queue the IR buffer for the audio thread to load (non-blocking)
        queueIRBuffer(IRBufferWithSampleRate(std::move(irBuffer), irSampleRate, irChannels));
        
        juce::Logger::writeToLog("  Buffer queued - audio thread will load on next processBlock()");
        lastLoadedIRPath = irFile.getFullPathName();
        
        juce::Logger::writeToLog("=== ConvolutionEngine::loadImpulseResponse END (SUCCESS - queued) ===");
        return true;
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("  EXCEPTION during loadImpulseResponse: " + juce::String(e.what()));
        irLoaded = false;
        return false;
    }
}

bool ConvolutionEngine::loadImpulseResponseFromMemory (const void* data, size_t size)
{
    DBG("=== loadImpulseResponseFromMemory START ===");
    convolver.loadImpulseResponse (data,
                                   size,
                                   juce::dsp::Convolution::Stereo::yes,
                                   juce::dsp::Convolution::Trim::yes,
                                   0,
                                   juce::dsp::Convolution::Normalise::yes);
    
    irLoaded = true;
    DBG("=== loadImpulseResponseFromMemory END (SUCCESS) ===");
    return true;
}
