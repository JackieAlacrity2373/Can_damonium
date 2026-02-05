#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
// Simple standalone UI component with audio routing
class DebugUIComponent : public juce::Component,
                        public juce::Timer
{
public:
    explicit DebugUIComponent(PluginProcessor& proc)
        : processor(proc), framesProcessed(0)
    {
        DBG("=== DebugUIComponent CONSTRUCTOR ===");
        
        // Initialize processor at default sample rate
        processor.prepareToPlay(48000.0, 512);
        DBG("  Processor prepared at 48kHz, 512 sample buffer");
        
        // Title label
        titleLabel = std::make_unique<juce::Label>("Title", "Can Damonium - Standalone Audio Test");
        titleLabel->setFont(juce::Font(18.0f, juce::Font::bold));
        addAndMakeVisible(*titleLabel);
        
        // Status display
        statusLabel = std::make_unique<juce::Label>("Status", 
            "Status: Ready\n"
            "IR: Loading...\n"
            "Audio: Waiting for input");
        statusLabel->setFont(juce::Font(12.0f));
        statusLabel->setColour(juce::Label::textColourId, juce::Colours::lightgreen);
        addAndMakeVisible(*statusLabel);
        
        // Info text
        infoLabel = std::make_unique<juce::Label>("Info", 
            "Standalone audio test harness:\n"
            "• Audio routing is configured for demonstration\n"
            "• Impulse response convolution is active\n"
            "• Check console for diagnostic messages\n"
            "\n"
            "Note: Full audio device selection requires additional setup.\n"
            "Currently routing through default system audio.");
        infoLabel->setFont(juce::Font(11.0f));
        infoLabel->setJustificationType(juce::Justification::topLeft);
        addAndMakeVisible(*infoLabel);
        
        setSize(650, 450);
        
        // Start status update timer
        startTimer(500);
        
        DBG("  DebugUIComponent setup complete");
    }
    
    ~DebugUIComponent() override
    {
        DBG("=== DebugUIComponent DESTRUCTOR ===");
        stopTimer();
        processor.releaseResources();
    }
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff2a2a2a));
        g.setColour(juce::Colours::white);
        g.setFont(10.0f);
        g.drawText("Frames processed: " + juce::String(framesProcessed), 
                  10, getHeight() - 25, getWidth() - 20, 20, juce::Justification::bottomRight);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(20);
        
        titleLabel->setBounds(bounds.removeFromTop(40));
        bounds.removeFromTop(10);
        
        statusLabel->setBounds(bounds.removeFromTop(100));
        bounds.removeFromTop(15);
        
        infoLabel->setBounds(bounds.removeFromTop(200));
    }
    
    void timerCallback() override
    {
        // Update UI periodically
        repaint(getWidth() - 300, getHeight() - 30, 280, 25);
    }
    
    PluginProcessor& getProcessor() { return processor; }
    
    void updateFrameCount(int64_t count)
    {
        framesProcessed = count;
    }

private:
    PluginProcessor& processor;
    int64_t framesProcessed;
    
    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::Label> infoLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DebugUIComponent)
};
