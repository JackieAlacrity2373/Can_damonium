#include "ConvolutionEngine.h"

namespace
{
    juce::AudioBuffer<float> resampleIrBuffer(const juce::AudioBuffer<float>& input,
                                              double sourceSampleRate,
                                              double targetSampleRate)
    {
        const int inputSamples = input.getNumSamples();
        const int channels = input.getNumChannels();
        const double ratio = sourceSampleRate / targetSampleRate;
        const int outputSamples = static_cast<int>(std::ceil(inputSamples / ratio));

        juce::AudioBuffer<float> output(channels, outputSamples);

        for (int ch = 0; ch < channels; ++ch)
        {
            juce::LagrangeInterpolator resampler;
            resampler.reset();
            resampler.process(ratio,
                              input.getReadPointer(ch),
                              output.getWritePointer(ch),
                              outputSamples);
        }

        return output;
    }
}

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
    
    // Don't reset - the JUCE convolver doesn't need reset after IR load
    // Calling reset clears important internal state
    // if (needsReset.load())
    // {
    //     convolver.reset();
    //     needsReset.store(false);
    //     juce::Logger::writeToLog(">>> Convolver reset on audio thread after IR load");
    // }
    
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
    juce::Logger::writeToLog("=== ConvolutionEngine::loadImpulseResponse START ===");
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

            if (resampleIrToDevice.load())
            {
                juce::Logger::writeToLog("  Matching IR to device rate by resampling...");

                irBuffer = resampleIrBuffer(irBuffer, irSampleRate, currentSampleRate);
                irSampleRate = currentSampleRate;

                juce::Logger::writeToLog("  Resampled IR: " + juce::String(irBuffer.getNumSamples()) +
                                         " samples at " + juce::String((int)irSampleRate) + " Hz");
            }
            else
            {
                juce::Logger::writeToLog("  Resample disabled - loading IR at original rate.");
            }
        }

        juce::Logger::writeToLog("  File loaded into buffer, now loading into convolver...");

        // Self-test: run a local convolver on a constant signal to verify sustained output
        // Important: Use the DEVICE sample rate for test, not the IR file sample rate
        if (currentSampleRate > 0.0 && currentBlockSize > 0)
        {
            juce::AudioBuffer<float> irBufferForTest;
            irBufferForTest.makeCopyOf (irBuffer);

            juce::dsp::Convolution testConvolver;
            juce::dsp::ProcessSpec testSpec;
            testSpec.sampleRate = currentSampleRate;  // Use device SR, not file SR
            testSpec.maximumBlockSize = static_cast<juce::uint32> (currentBlockSize);
            testSpec.numChannels = static_cast<juce::uint32> (irBuffer.getNumChannels());
            testConvolver.prepare (testSpec);

            testConvolver.loadImpulseResponse (std::move (irBufferForTest),
                                               irSampleRate,  // Source IR sample rate  
                                               irChannels == 2 ? juce::dsp::Convolution::Stereo::yes : juce::dsp::Convolution::Stereo::no,
                                               juce::dsp::Convolution::Trim::no,
                                               juce::dsp::Convolution::Normalise::yes);

            juce::AudioBuffer<float> testBlock (irBuffer.getNumChannels(), currentBlockSize);

            for (int i = 0; i < 10; ++i)
            {
                testBlock.clear();
                if (i == 0)
                {
                    for (int ch = 0; ch < testBlock.getNumChannels(); ++ch)
                        testBlock.setSample (ch, 0, 1.0f);
                }

                juce::dsp::AudioBlock<float> testAudioBlock (testBlock);
                juce::dsp::ProcessContextReplacing<float> testContext (testAudioBlock);
                testConvolver.process (testContext);

                float testRms = 0.0f;
                for (int ch = 0; ch < testBlock.getNumChannels(); ++ch)
                    testRms = juce::jmax (testRms, testBlock.getRMSLevel (ch, 0, testBlock.getNumSamples()));

                juce::Logger::writeToLog ("  SelfTest block #" + juce::String (i + 1) + " RMS=" + juce::String (testRms, 6));
            }
        }
        else
        {
            juce::Logger::writeToLog ("  SelfTest skipped (invalid sample rate/block size)");
        }

        // Load IR into main convolver
        convolver.loadImpulseResponse(std::move(irBuffer),
                          irSampleRate,  // Source IR sample rate
                          irChannels == 2 ? juce::dsp::Convolution::Stereo::yes : juce::dsp::Convolution::Stereo::no,
                          juce::dsp::Convolution::Trim::no,
                          juce::dsp::Convolution::Normalise::yes);

        juce::Logger::writeToLog("  SUCCESS: IR loaded into convolver (" + irFile.getFileName() + ")");
        
        // Do NOT reset here - let the next processBlock do the reset if needed
        
        lastLoadedIRPath = irFile.getFullPathName();
        irLoaded.store(true);
        
        juce::Logger::writeToLog("  irLoaded flag set to: true");
        juce::Logger::writeToLog("=== ConvolutionEngine::loadImpulseResponse END (SUCCESS) ===");
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
