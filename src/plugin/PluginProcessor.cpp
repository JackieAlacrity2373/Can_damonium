#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../common/Constants.h"
#include "IRLibraryManager.h"

PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    DBG("=== PluginProcessor CONSTRUCTOR START ===");
    convolutionEngine = std::make_unique<ConvolutionEngine>();
    DBG("=== PluginProcessor CONSTRUCTOR END - ConvolutionEngine created ===");
}

PluginProcessor::~PluginProcessor()
{
}

void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::Logger::writeToLog (">>> PluginProcessor::prepareToPlay START (SR:" + juce::String(sampleRate) + " BS:" + juce::String(samplesPerBlock) + ")");

    prepareToPlayCount.fetch_add (1, std::memory_order_relaxed);

    currentSampleRateHz.store (sampleRate);
    currentBlockSize.store (samplesPerBlock);
    
    convolutionEngine->prepareToPlay (sampleRate, samplesPerBlock);
    juce::Logger::writeToLog (">>> PluginProcessor::prepareToPlay - ConvolutionEngine prepared");
    
    // Don't load IR here - it causes blocking file I/O on audio thread
    // IR will be loaded later from the message thread
    
    bool irLoaded = (convolutionEngine != nullptr && convolutionEngine->isIrLoaded());
    juce::Logger::writeToLog (">>> PluginProcessor::prepareToPlay END - IR Loaded: " + juce::String(irLoaded ? "true" : "false"));
}

void PluginProcessor::releaseResources()
{
    // Convolution engine handles cleanup
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    lastLayoutInputs.store (layouts.getMainInputChannelSet().size(), std::memory_order_relaxed);
    lastLayoutOutputs.store (mainOut.size(), std::memory_order_relaxed);
    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

    const auto& mainIn = layouts.getMainInputChannelSet();
    if (mainIn != juce::AudioChannelSet::disabled()
        && mainIn != juce::AudioChannelSet::mono()
        && mainIn != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn != juce::AudioChannelSet::disabled()
        && mainIn.size() > mainOut.size())
        return false;

    return true;
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    static int logCount = 0;
    bool shouldLog = (logCount++ < 3);
    
    if (shouldLog)
        juce::Logger::writeToLog (">>> PluginProcessor::processBlock #" + juce::String(logCount) + " START");
    
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    audioCallbackCount.fetch_add (1, std::memory_order_relaxed);

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    if (shouldLog)
    {
        juce::Logger::writeToLog (">>> processBlock channels: in=" + juce::String(totalNumInputChannels) + " out=" + juce::String(totalNumOutputChannels));
        juce::Logger::writeToLog (">>> processBlock buffer channels: " + juce::String(buffer.getNumChannels()) + " samples: " + juce::String(buffer.getNumSamples()));
    }

    // Input level (pre-processing)
    float inLevel = 0.0f;
    for (int ch = 0; ch < juce::jmin(totalNumInputChannels, buffer.getNumChannels()); ++ch)
        inLevel = juce::jmax (inLevel, buffer.getRMSLevel (ch, 0, buffer.getNumSamples()));
    inputLevel.store (inLevel);

    // Generate test tone if enabled (for debugging convolution)
    if (testToneEnabled.load())
    {
        const double phaseIncrement = juce::MathConstants<double>::twoPi * testToneFrequency / currentSampleRateHz.load();
        
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);
            double phase = testTonePhase;
            
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                channelData[i] = 0.3f * std::sin(static_cast<float>(phase)); // 30% amplitude
                phase += phaseIncrement;
                while (phase >= juce::MathConstants<double>::twoPi)
                    phase -= juce::MathConstants<double>::twoPi;
            }
        }
        
        testTonePhase += phaseIncrement * buffer.getNumSamples();
        while (testTonePhase >= juce::MathConstants<double>::twoPi)
            testTonePhase -= juce::MathConstants<double>::twoPi;
        
        // Update input level with test tone
        inLevel = 0.3f;
        inputLevel.store(inLevel);
    }

    if (shouldLog)
        juce::Logger::writeToLog (">>> processBlock input level: " + juce::String(inLevel, 4));

    // Apply convolution to audio buffer (or pass through if bypassed/no IR)
    if (convolutionEngine)
    {
        bool irLoaded = convolutionEngine->isIrLoaded();
        bool bypassed = convolutionEngine->isBypassed();
        
        if (shouldLog)
            juce::Logger::writeToLog (">>> ConvEngine state: IR=" + juce::String(irLoaded ? "loaded" : "none") + " Bypass=" + juce::String(bypassed ? "ON" : "OFF"));
        
        if (!irLoaded && inLevel > 0.001f)
        {
            DBG("WARNING: Audio present but IR not loaded!");
        }
        convolutionEngine->processBlock (buffer);
    }
    else
    {
        // No convolution engine - ensure passthrough
        // Buffer already contains input, so we don't need to do anything
        if (shouldLog)
            juce::Logger::writeToLog (">>> No convolution engine - passthrough");
    }

    // Convolution level (post-convolution/passthrough)
    float convLevel = 0.0f;
    for (int ch = 0; ch < juce::jmin(totalNumOutputChannels, buffer.getNumChannels()); ++ch)
        convLevel = juce::jmax (convLevel, buffer.getRMSLevel (ch, 0, buffer.getNumSamples()));
    convolutionLevel.store (convLevel);

    // Output level (same as post-convolution for now)
    outputLevel.store (convLevel);
    
    if (shouldLog)
        juce::Logger::writeToLog (">>> processBlock output level: " + juce::String(convLevel, 4));
    
    // Log if we have input but no output when not bypassed
    static int noOutputWarningCount = 0;
    if (inLevel > 0.001f && convLevel < 0.0001f && convolutionEngine && !convolutionEngine->isBypassed() && noOutputWarningCount++ < 5)
    {
        juce::Logger::writeToLog("!!! WARNING: Input present (" + juce::String(inLevel, 4) + ") but output silent (" + 
                                 juce::String(convLevel, 4) + ") - warning #" + juce::String(noOutputWarningCount));
    }
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = std::make_unique<juce::XmlElement>("Can_damonium");
    state->setAttribute ("version", "1.0");
    copyXmlToBinary (*state, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto state = getXmlFromBinary (data, sizeInBytes);
    if (state != nullptr && state->hasTagName ("Can_damonium"))
    {
        // Restore state from XML
    }
}

void PluginProcessor::loadImpulseResponse (const juce::File& irFile)
{
    DBG("=== PluginProcessor::loadImpulseResponse START ===");
    DBG("  File: " + irFile.getFullPathName());
    
    if (!irFile.existsAsFile())
    {
        DBG("  ERROR: File does not exist!");
        return;
    }
    
    try
    {
        if (convolutionEngine)
        {
            // Direct load - JUCE Convolution handles thread safety internally
            bool success = convolutionEngine->loadImpulseResponse (irFile);
            DBG("  Load returned: " << (success ? "true" : "false"));
            DBG("  IR status after load - isIrLoaded(): " << (convolutionEngine->isIrLoaded() ? "true" : "false"));
        }
        else
        {
            DBG("  ERROR: ConvolutionEngine is null!");
        }
    }
    catch (const std::exception& e)
    {
        DBG("  EXCEPTION: " + juce::String(e.what()));
    }
    
    DBG("=== PluginProcessor::loadImpulseResponse END ===");
}

void PluginProcessor::loadPresetProfile (const juce::String& profileName)
{
    DBG("=== loadPresetProfile START: " + profileName + " ===");
    
    // Try to load from application resources directory
    auto appDataPath = juce::File::getSpecialLocation (juce::File::currentApplicationFile)
                            .getParentDirectory()
                            .getParentDirectory();
    
    DBG("  appDataPath: " + appDataPath.getFullPathName());
    
    juce::String fileName = "RegularPringlesCan48k.wav";
    if (profileName == CanDamonium::PROFILE_SMALL)
        fileName = "SmallPringlesCan48k.wav";
    else if (profileName == CanDamonium::PROFILE_GRANDE)
        fileName = "GrandePringlesCan48k.wav";
    
    DBG("  Looking for file: " + fileName);
    
    // Try common resource locations
    juce::File resourceDir = appDataPath.getChildFile ("Resources");
    DBG("  Trying: " + resourceDir.getFullPathName());
    if (!resourceDir.exists())
    {
        resourceDir = appDataPath.getChildFile ("Data");
        DBG("  Trying: " + resourceDir.getFullPathName());
    }
    if (!resourceDir.exists())
    {
        resourceDir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                        .getChildFile ("Can_damonium/IRs");
        DBG("  Trying: " + resourceDir.getFullPathName());
    }
    
    juce::File irFile = resourceDir.getChildFile (fileName);
    
    DBG("  Final IR path: " + irFile.getFullPathName());
    DBG("  File exists: " + juce::String(irFile.existsAsFile() ? "true" : "false"));
    
    if (irFile.existsAsFile())
    {
        DBG("  SUCCESS: Loading IR from: " + irFile.getFullPathName());
        convolutionEngine->loadImpulseResponse (irFile);
        DBG("=== loadPresetProfile END (SUCCESS) ===");
    }
    else
    {
        DBG("  FAILED: Preset profile not found: " + irFile.getFullPathName());
        DBG("=== loadPresetProfile END (FAILED) ===");
    }
}

void PluginProcessor::saveCurrentIRToLibrary (const juce::String& fileName)
{
    // TODO: Implement saving IR to user library
}

// Audio processor factory
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
