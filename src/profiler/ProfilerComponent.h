#pragma once

#include <JuceHeader.h>
#include "TestSignalGenerator.h"

/**
 * Main UI for the profiler application
 * Allows recording impulse responses from amplifiers
 */
class ProfilerComponent : public juce::Component,
                          public juce::Timer,
                          public juce::ComboBox::Listener,
                          public juce::Button::Listener
{
public:
    ProfilerComponent();
    ~ProfilerComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Listener callbacks
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (juce::Button* button) override;

private:
    // Components
    std::unique_ptr<juce::ComboBox> profileSelector;
    std::unique_ptr<juce::ComboBox> testSignalSelector;
    std::unique_ptr<juce::TextButton> playButton;
    std::unique_ptr<juce::TextButton> recordButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::Label> statusLabel;
    juce::ProgressBar progressBar;
    std::unique_ptr<juce::Label> levelMeterLabel;

    // State
    TestSignalGenerator signalGenerator;
    bool isPlaying = false;
    bool isRecording = false;
    double recordingProgress = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfilerComponent)
};
