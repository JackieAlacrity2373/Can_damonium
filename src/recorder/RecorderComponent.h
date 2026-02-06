#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "TestSignalGenerator.h"
#include "InstructionPanel.h"

/**
 * Main UI component for IR recording guidance application
 * Guides user through recording impulse responses from their equipment
 */
class RecorderComponent : public juce::AudioAppComponent,
                          public juce::Timer,
                          public juce::Button::Listener,
                          public juce::ComboBox::Listener
{
public:
    RecorderComponent();
    ~RecorderComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    // Audio callbacks
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Button callbacks
    void buttonClicked (juce::Button* button) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    
    // Menu callback
    void showAudioSettings();

private:
    // UI Components
    std::unique_ptr<InstructionPanel> instructionPanel;
    std::unique_ptr<juce::ComboBox> equipmentSelector;
    std::unique_ptr<juce::Label> equipmentLabel;
    std::unique_ptr<juce::TextButton> nextButton;
    std::unique_ptr<juce::TextButton> backButton;
    std::unique_ptr<juce::TextButton> recordButton;
    std::unique_ptr<juce::TextButton> stopButton;
    std::unique_ptr<juce::TextButton> moreIRsButton;
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::Label> levelMeterLabel;
    juce::ProgressBar progressBar;

    // State machine
    InstructionPanel::Step currentStep = InstructionPanel::Step::Setup;

    // Recording state
    bool isRecording = false;
    bool isPlaying = false;
    juce::AudioBuffer<float> recordingBuffer;
    int recordingPosition = 0;
    double recordingProgress = 0.0;
    double currentSampleRate = 48000.0;

    // Test signal generation
    TestSignalGenerator signalGenerator;

    // File management
    juce::File libraryDirectory;
    void initializeLibraryDirectory();
    void saveRecordingToFile (const juce::String& equipmentName, const juce::AudioBuffer<float>& bufferToSave);
    void validateAndProcessRecording();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecorderComponent)
};
