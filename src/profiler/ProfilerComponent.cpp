#include "ProfilerComponent.h"

ProfilerComponent::ProfilerComponent()
    : progressBar (recordingProgress)
{
    // Profile selector
    profileSelector = std::make_unique<juce::ComboBox>();
    profileSelector->addItem ("Small (Pringles Can)", 1);
    profileSelector->addItem ("Regular (Pringles Can)", 2);
    profileSelector->addItem ("Grande (Pringles Can)", 3);
    profileSelector->setSelectedItemIndex (0);
    profileSelector->addListener (this);
    addAndMakeVisible (*profileSelector);

    // Test signal selector
    testSignalSelector = std::make_unique<juce::ComboBox>();
    testSignalSelector->addItem ("Logarithmic Chirp (20Hz-20kHz, 10s)", 1);
    testSignalSelector->addItem ("White Noise (4s)", 2);
    testSignalSelector->setSelectedItemIndex (0);
    testSignalSelector->addListener (this);
    addAndMakeVisible (*testSignalSelector);

    // Buttons
    playButton = std::make_unique<juce::TextButton> ("Play Test Signal");
    playButton->addListener (this);
    addAndMakeVisible (*playButton);

    recordButton = std::make_unique<juce::TextButton> ("Start Recording");
    recordButton->addListener (this);
    addAndMakeVisible (*recordButton);

    stopButton = std::make_unique<juce::TextButton> ("Stop");
    stopButton->addListener (this);
    stopButton->setEnabled (false);
    addAndMakeVisible (*stopButton);

    // Status display
    statusLabel = std::make_unique<juce::Label> ("Status", "Ready");
    addAndMakeVisible (*statusLabel);

    levelMeterLabel = std::make_unique<juce::Label> ("Level", "Level: 0 dB");
    addAndMakeVisible (*levelMeterLabel);

    addAndMakeVisible (progressBar);

    // Initialize signal generator at 48 kHz
    signalGenerator.prepareToPlay (48000.0);

    startTimer (50); // Update UI every 50ms
}

ProfilerComponent::~ProfilerComponent()
{
    stopTimer();
}

void ProfilerComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    g.drawText ("Can Damonium IR Profiler", 10, 10, getWidth() - 20, 30, juce::Justification::centredTop);

    g.setFont (14.0f);
    g.drawText ("Profile to Record:", 10, 50, 150, 25, juce::Justification::bottomLeft);
    g.drawText ("Test Signal:", 10, 95, 150, 25, juce::Justification::bottomLeft);
}

void ProfilerComponent::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    auto row1 = bounds.removeFromTop (40);
    profileSelector->setBounds (160, 50, 400, 25);

    auto row2 = bounds.removeFromTop (40);
    testSignalSelector->setBounds (160, 95, 400, 25);

    auto row3 = bounds.removeFromTop (50);
    playButton->setBounds (row3.removeFromLeft (150).reduced (5));
    recordButton->setBounds (row3.removeFromLeft (150).reduced (5));
    stopButton->setBounds (row3.removeFromLeft (150).reduced (5));

    auto row4 = bounds.removeFromTop (30);
    statusLabel->setBounds (row4);

    auto row5 = bounds.removeFromTop (30);
    levelMeterLabel->setBounds (row5);

    auto row6 = bounds.removeFromTop (20);
    progressBar.setBounds (row6.reduced (5));
}

void ProfilerComponent::timerCallback()
{
    // Update level meter
    float level = 0.0f; // TODO: Get from IRRecorder
    float levelDb = 20.0f * std::log10 (level + 0.0001f);
    levelMeterLabel->setText ("Level: " + juce::String (levelDb, 1) + " dB", juce::dontSendNotification);

    // Update progress
    recordingProgress = signalGenerator.getProgress();
    repaint();
}

void ProfilerComponent::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == profileSelector.get())
    {
        statusLabel->setText ("Profile selected: " + profileSelector->getText(), juce::dontSendNotification);
    }
    else if (comboBoxThatHasChanged == testSignalSelector.get())
    {
        statusLabel->setText ("Test signal selected: " + testSignalSelector->getText(), juce::dontSendNotification);
    }
}

void ProfilerComponent::buttonClicked (juce::Button* button)
{
    if (button == playButton.get())
    {
        isPlaying = !isPlaying;
        if (isPlaying)
        {
            signalGenerator.reset();
            playButton->setButtonText ("Stop Playback");
            statusLabel->setText ("Playing test signal...", juce::dontSendNotification);
            recordButton->setEnabled (false);
        }
        else
        {
            playButton->setButtonText ("Play Test Signal");
            statusLabel->setText ("Stopped", juce::dontSendNotification);
            recordButton->setEnabled (true);
        }
    }
    else if (button == recordButton.get())
    {
        isRecording = true;
        recordButton->setEnabled (false);
        playButton->setEnabled (false);
        stopButton->setEnabled (true);
        statusLabel->setText ("Recording IR from " + profileSelector->getText() + "...", juce::dontSendNotification);
        signalGenerator.reset();
    }
    else if (button == stopButton.get())
    {
        isRecording = false;
        recordButton->setEnabled (true);
        playButton->setEnabled (true);
        stopButton->setEnabled (false);
        statusLabel->setText ("Recording stopped. Saved to library.", juce::dontSendNotification);
    }
}
