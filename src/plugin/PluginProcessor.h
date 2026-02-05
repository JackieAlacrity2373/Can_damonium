#pragma once

#include <JuceHeader.h>
#include "ConvolutionEngine.h"
#include "IRLibraryManager.h"

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return "Can Damonium"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override { juce::ignoreUnused (index); }
    const juce::String getProgramName (int index) override { return "Default"; }
    void changeProgramName (int index, const juce::String& newName) override { juce::ignoreUnused (index, newName); }

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // IR Management
    void loadImpulseResponse (const juce::File& irFile);
    void setDeferredIRLoad (const juce::File& irFile) noexcept 
    { 
        if (convolutionEngine) 
            convolutionEngine->setDeferredIRLoad(irFile); 
    }
    void loadPresetProfile (const juce::String& profileName);
    void saveCurrentIRToLibrary (const juce::String& fileName);
    
    IRLibraryManager& getIRLibrary() noexcept { return irLibrary; }
    bool isIrLoaded() const noexcept { return convolutionEngine != nullptr && convolutionEngine->isIrLoaded(); }
    double getCurrentSampleRateHz() const noexcept { return currentSampleRateHz.load(); }
    int getCurrentBlockSize() const noexcept { return currentBlockSize.load(); }

    //==============================================================================
    // Bypass control
    void setConvolutionBypass (bool shouldBypass) noexcept
    {
        if (convolutionEngine)
            convolutionEngine->setBypass (shouldBypass);
    }
    
    bool isConvolutionBypassed() const noexcept
    {
        return convolutionEngine ? convolutionEngine->isBypassed() : false;
    }
    
    //==============================================================================
    // Test tone generator (for debugging)
    void setTestToneEnabled (bool enabled) noexcept { testToneEnabled.store(enabled); }
    bool isTestToneEnabled() const noexcept { return testToneEnabled.load(); }

    //==============================================================================
    // Level meters (linear 0.0 - 1.0)
    float getInputLevel() const noexcept { return inputLevel.load(); }
    float getConvolutionLevel() const noexcept { return convolutionLevel.load(); }
    float getOutputLevel() const noexcept { return outputLevel.load(); }

    // Audio callback diagnostics
    uint64_t getAudioCallbackCount() const noexcept { return audioCallbackCount.load(); }
    int getPrepareToPlayCount() const noexcept { return prepareToPlayCount.load(); }
    int getLastLayoutInputs() const noexcept { return lastLayoutInputs.load(); }
    int getLastLayoutOutputs() const noexcept { return lastLayoutOutputs.load(); }

private:
    std::unique_ptr<ConvolutionEngine> convolutionEngine;
    IRLibraryManager irLibrary;

    std::atomic<double> currentSampleRateHz { 0.0 };
    std::atomic<int> currentBlockSize { 0 };

    std::atomic<float> inputLevel { 0.0f };
    std::atomic<float> convolutionLevel { 0.0f };
    std::atomic<float> outputLevel { 0.0f };

    std::atomic<uint64_t> audioCallbackCount { 0 };
    std::atomic<int> prepareToPlayCount { 0 };
    mutable std::atomic<int> lastLayoutInputs { -1 };
    mutable std::atomic<int> lastLayoutOutputs { -1 };
    
    // Test tone generation
    std::atomic<bool> testToneEnabled { false };
    double testTonePhase { 0.0 };
    double testToneFrequency { 440.0 }; // A4 note

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
