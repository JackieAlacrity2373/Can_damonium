#include "RecorderComponent.h"
#include <cmath>

RecorderComponent::RecorderComponent()
    : progressBar (recordingProgress)
{
    // Initialize library directory
    initializeLibraryDirectory();

    // Instruction panel
    instructionPanel = std::make_unique<InstructionPanel>();
    instructionPanel->setCurrentStep (currentStep);
    addAndMakeVisible (*instructionPanel);

    // Equipment selector (only visible on Step 1)
    equipmentLabel = std::make_unique<juce::Label> ("Equipment", "Equipment/Amp Name:");
    addAndMakeVisible (*equipmentLabel);

    equipmentSelector = std::make_unique<juce::ComboBox>();
    equipmentSelector->addItem ("Small Pringles Can", 1);
    equipmentSelector->addItem ("Regular Pringles Can", 2);
    equipmentSelector->addItem ("Grande Pringles Can", 3);
    equipmentSelector->addItem ("Custom Equipment", 4);
    equipmentSelector->setSelectedItemIndex (0);
    equipmentSelector->addListener (this);
    addAndMakeVisible (*equipmentSelector);

    // Navigation buttons
    backButton = std::make_unique<juce::TextButton> ("BACK");
    backButton->addListener (this);
    backButton->setEnabled (false);
    addAndMakeVisible (*backButton);

    nextButton = std::make_unique<juce::TextButton> ("NEXT");
    nextButton->addListener (this);
    addAndMakeVisible (*nextButton);

    recordButton = std::make_unique<juce::TextButton> ("RECORD");
    recordButton->addListener (this);
    recordButton->setEnabled (false);
    addAndMakeVisible (*recordButton);

    stopButton = std::make_unique<juce::TextButton> ("STOP");
    stopButton->addListener (this);
    stopButton->setEnabled (false);
    addAndMakeVisible (*stopButton);

    moreIRsButton = std::make_unique<juce::TextButton> ("MORE IRs");
    moreIRsButton->addListener (this);
    moreIRsButton->setEnabled (true);
    addAndMakeVisible (*moreIRsButton);

    // Status display
    statusLabel = std::make_unique<juce::Label> ("Status", "Ready");
    addAndMakeVisible (*statusLabel);

    levelMeterLabel = std::make_unique<juce::Label> ("Level", "Input Level: -∞ dB");
    addAndMakeVisible (*levelMeterLabel);

    addAndMakeVisible (progressBar);

    // Set up audio channels (AudioAppComponent manages the device)
    setAudioChannels (1, 1);
    
    // Auto-select Focusrite devices if available
    auto& deviceManager = AudioAppComponent::deviceManager;
    auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
    
    if (deviceType)
    {
        auto inputDevices = deviceType->getDeviceNames (true);
        auto outputDevices = deviceType->getDeviceNames (false);
        
        juce::String preferredInput, preferredOutput;
        
        // Find Focusrite Analogue input
        for (const auto& name : inputDevices)
            if (name.containsIgnoreCase("Focusrite") && name.containsIgnoreCase("Analogue"))
                preferredInput = name;
        
        // Find Focusrite USB or S/PDIF output  
        for (const auto& name : outputDevices)
            if (name.containsIgnoreCase("Focusrite"))
                preferredOutput = name;
        
        if (preferredInput.isNotEmpty() || preferredOutput.isNotEmpty())
        {
            auto setup = deviceManager.getAudioDeviceSetup();
            if (preferredInput.isNotEmpty()) setup.inputDeviceName = preferredInput;
            if (preferredOutput.isNotEmpty()) setup.outputDeviceName = preferredOutput;
            deviceManager.setAudioDeviceSetup (setup, true);
            DBG("Auto-selected Focusrite devices");
        }
    }

    // Start UI update timer
    startTimer (50);
}

RecorderComponent::~RecorderComponent()
{
    stopTimer();
    shutdownAudio();
}

void RecorderComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    signalGenerator.prepareToPlay (sampleRate);
    
    // Allocate recording buffer for actual sample rate (14 seconds)
    recordingBuffer.setSize (1, static_cast<int>(sampleRate * 14));
    
    DBG("RecorderComponent prepared: SR=" + juce::String((int)sampleRate) + 
        " BS=" + juce::String(samplesPerBlockExpected));
}

void RecorderComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Title
    g.setColour (juce::Colours::white);
    g.setFont (22.0f);
    g.drawText ("Can Damonium IR Recorder", 10, 10, getWidth() - 20, 30, juce::Justification::centredTop);

    // Step indicator
    g.setFont (12.0f);
    g.drawText ("Step " + juce::String ((int)currentStep + 1) + " of 5", 10, 40, 100, 20, juce::Justification::topLeft);
}

void RecorderComponent::resized()
{
    auto bounds = getLocalBounds().reduced (10);

    // Title area
    bounds.removeFromTop (60);

    // Equipment selector (visible on Setup step only)
    if (currentStep == InstructionPanel::Step::Setup)
    {
        auto equipRow = bounds.removeFromTop (40);
        equipmentLabel->setBounds (equipRow.removeFromLeft (150).reduced (5));
        equipmentSelector->setBounds (equipRow.reduced (5));
    }

    // Instruction panel (main content area)
    instructionPanel->setBounds (bounds.removeFromTop (getHeight() - 200));

    // Bottom controls
    auto buttonRow = bounds.removeFromBottom (40);
    auto statusRow = bounds;

    // Buttons - layout depends on current step
    auto btnWidth = 100;
    
    if (currentStep == InstructionPanel::Step::Complete)
    {
        // Show only nextButton (renamed to "NEW RECORDING") and moreIRsButton on Complete step
        nextButton->setBounds (buttonRow.removeFromLeft (btnWidth).reduced (5));
        moreIRsButton->setBounds (buttonRow.removeFromLeft (btnWidth).reduced (5));
        moreIRsButton->setVisible (true);
        backButton->setVisible (false);
        recordButton->setVisible (false);
        stopButton->setVisible (false);
    }
    else
    {
        // Normal step navigation
        backButton->setBounds (buttonRow.removeFromLeft (btnWidth).reduced (5));
        nextButton->setBounds (buttonRow.removeFromLeft (btnWidth).reduced (5));
        recordButton->setBounds (buttonRow.removeFromLeft (btnWidth).reduced (5));
        stopButton->setBounds (buttonRow.removeFromLeft (btnWidth).reduced (5));
        moreIRsButton->setVisible (false);
        backButton->setVisible (true);
        recordButton->setVisible (true);
        stopButton->setVisible (true);
    }

    // Status area
    statusLabel->setBounds (statusRow.removeFromTop (15).reduced (5));
    levelMeterLabel->setBounds (statusRow.removeFromTop (15).reduced (5));
    progressBar.setBounds (statusRow.reduced (5));
}

void RecorderComponent::timerCallback()
{
    // Update level meter
    if (isRecording && recordingPosition > 0)
    {
        auto* data = recordingBuffer.getReadPointer (0);
        float maxLevel = 0.0f;

        int checkSamples = juce::jmin (recordingPosition, static_cast<int>(currentSampleRate * 0.05)); // Last 50ms
        for (int i = recordingPosition - checkSamples; i < recordingPosition; ++i)
            maxLevel = juce::jmax (maxLevel, std::abs (data[i]));

        float levelDb = (maxLevel > 0.0f) ? 20.0f * std::log10 (maxLevel) : -100.0f;
        levelMeterLabel->setText ("Input Level: " + juce::String (levelDb, 1) + " dB", juce::dontSendNotification);
    }

    // Update progress
    if (isRecording)
    {
        recordingProgress = static_cast<double> (recordingPosition) / recordingBuffer.getNumSamples();

        if (recordingPosition >= recordingBuffer.getNumSamples())
        {
            isRecording = false;
            isPlaying = false;
            recordButton->setEnabled (true);
            stopButton->setEnabled (false);
            statusLabel->setText ("Recording complete. Processing...", juce::dontSendNotification);
            DBG("Recording complete, validating...");
            validateAndProcessRecording();
        }
    }

    repaint();
}

void RecorderComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* outputData = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* inputData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);

    // Generate test signal if playing
    if (isPlaying && currentStep == InstructionPanel::Step::Recording)
    {
        signalGenerator.generateLogarithmicChirp (*bufferToFill.buffer, bufferToFill.startSample, bufferToFill.numSamples);
    }
    else
    {
        std::fill (outputData, outputData + bufferToFill.numSamples, 0.0f);
    }

    // Record input
    if (isRecording && recordingPosition < recordingBuffer.getNumSamples())
    {
        auto* recordData = recordingBuffer.getWritePointer (0);
        int samplesToRecord = bufferToFill.numSamples;

        if (recordingPosition + samplesToRecord > recordingBuffer.getNumSamples())
            samplesToRecord = recordingBuffer.getNumSamples() - recordingPosition;

        std::copy (inputData, inputData + samplesToRecord, recordData + recordingPosition);
        recordingPosition += samplesToRecord;
    }
}

void RecorderComponent::releaseResources()
{
}

void RecorderComponent::buttonClicked (juce::Button* button)
{
    if (button == nextButton.get())
    {
        if (currentStep == InstructionPanel::Step::Setup)
        {
            currentStep = InstructionPanel::Step::Configuration;
            backButton->setEnabled (true);
            recordButton->setEnabled (false);
            stopButton->setEnabled (false);
        }
        else if (currentStep == InstructionPanel::Step::Configuration)
        {
            currentStep = InstructionPanel::Step::Recording;
            recordButton->setEnabled (true);
        }

        instructionPanel->setCurrentStep (currentStep);
        resized();
        repaint();
    }
    else if (button == backButton.get())
    {
        if (currentStep == InstructionPanel::Step::Configuration)
        {
            currentStep = InstructionPanel::Step::Setup;
            backButton->setEnabled (false);
            recordButton->setEnabled (false);
        }
        else if (currentStep == InstructionPanel::Step::Recording)
        {
            currentStep = InstructionPanel::Step::Configuration;
        }

        instructionPanel->setCurrentStep (currentStep);
        resized();
        repaint();
    }
    else if (button == recordButton.get())
    {
        isRecording = true;
        isPlaying = true;
        recordingPosition = 0;
        recordingBuffer.clear();
        signalGenerator.reset();

        recordButton->setEnabled (false);
        stopButton->setEnabled (true);
        nextButton->setEnabled (false);
        backButton->setEnabled (false);

        statusLabel->setText ("Recording in progress...", juce::dontSendNotification);
    }
    else if (button == stopButton.get())
    {
        isRecording = false;
        isPlaying = false;
        stopButton->setEnabled (false);
        recordButton->setEnabled (true);
        nextButton->setEnabled (true);
        backButton->setEnabled (true);

        statusLabel->setText ("Recording stopped.", juce::dontSendNotification);
    }
    else if (button == moreIRsButton.get())
    {
        // Reset to Setup step to record another IR
        currentStep = InstructionPanel::Step::Setup;
        instructionPanel->setCurrentStep (currentStep);
        nextButton->setButtonText ("NEXT");
        nextButton->setEnabled (true);
        backButton->setEnabled (false);
        recordButton->setEnabled (false);
        stopButton->setEnabled (false);
        statusLabel->setText ("Ready to record another IR", juce::dontSendNotification);
        resized();
        repaint();
    }
}

void RecorderComponent::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == equipmentSelector.get())
    {
        statusLabel->setText ("Selected: " + equipmentSelector->getText(), juce::dontSendNotification);
    }
}

void RecorderComponent::showAudioSettings()
{
    auto& deviceManager = AudioAppComponent::deviceManager;
    
    DBG("=== showAudioSettings requested ===");
    auto* currentDevice = deviceManager.getCurrentAudioDevice();
    if (currentDevice)
    {
        DBG("Current device: " + currentDevice->getName());
        DBG("  Sample rate: " + juce::String((int)currentDevice->getCurrentSampleRate()));
    }
    
    // Create device selector component
    auto selector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        deviceManager,
        1,    // min input channels
        1,    // max input channels  
        1,    // min output channels
        1,    // max output channels
        false, // show MIDI input
        false, // show MIDI output
        false, // show channels as stereo pairs
        false  // hide advanced options
    );
    
    selector->setSize(500, 400);
    
    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(selector.release());
    options.dialogTitle = "Audio Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;
    
    options.launchAsync();
    
    DBG("Audio settings dialog launched");
}

void RecorderComponent::initializeLibraryDirectory()
{
    libraryDirectory = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Can_damonium")
        .getChildFile ("IRs");

    if (!libraryDirectory.exists())
        libraryDirectory.createDirectory();
}

void RecorderComponent::saveRecordingToFile (const juce::String& equipmentName, const juce::AudioBuffer<float>& bufferToSave)
{
    DBG("=== saveRecordingToFile START ===");
    
    // Create filename with timestamp
    auto now = juce::Time::getCurrentTime();
    juce::String sanitizedName = equipmentName;
    sanitizedName = sanitizedName.replace (" ", "_").replace ("(", "").replace (")", "");
    
    int sampleRateK = static_cast<int>(currentSampleRate / 1000);
    juce::String filename = sanitizedName + "_" 
        + now.formatted ("%Y%m%d_%H%M%S") 
        + "_" + juce::String(sampleRateK) + "k.wav";

    juce::File outputFile = libraryDirectory.getChildFile (filename);
    
    DBG("Output file: " + outputFile.getFullPathName());
    DBG("Buffer to save: " + juce::String(bufferToSave.getNumSamples()) + " samples");

    try
    {
        // Delete file if it exists
        if (outputFile.exists())
        {
            DBG("File exists, deleting...");
            outputFile.deleteFile();
        }
        
        // Create output stream
        auto outputStream = outputFile.createOutputStream();
        if (outputStream == nullptr || !outputStream->openedOk())
        {
            DBG ("Failed to open output stream");
            statusLabel->setText ("ERROR: Could not create file!", juce::dontSendNotification);
            return;
        }
        
        DBG("Output stream created successfully");

        // Create WAV audio format
        juce::WavAudioFormat wavFormat;

        DBG("Creating WAV writer at " + juce::String((int)currentSampleRate) + " Hz");
        
        // Create writer with actual sample rate
        auto writer = std::unique_ptr<juce::AudioFormatWriter> (
            wavFormat.createWriterFor (
                outputStream.release(),  // Transfer ownership to writer
                currentSampleRate,       // Use actual device sample rate
                1,                       // Channels (mono)
                24,                      // Bit depth
                {},                      // Metadata
                0                        // Options
            )
        );

        if (!writer)
        {
            DBG ("Failed to create WAV writer");
            statusLabel->setText ("ERROR: Could not create WAV writer!", juce::dontSendNotification);
            return;
        }

        DBG("Writing " + juce::String(bufferToSave.getNumSamples()) + " samples...");
        
        // Write audio buffer to file
        if (!writer->writeFromAudioSampleBuffer (bufferToSave, 0, bufferToSave.getNumSamples()))
        {
            DBG ("Failed to write audio data");
            statusLabel->setText ("ERROR: Failed to write audio data!", juce::dontSendNotification);
            return;
        }

        DBG("Flushing and closing writer...");
        
        // Flush and close by resetting writer
        writer.reset();

        DBG ("Successfully saved IR to: " + outputFile.getFullPathName());
        DBG ("File size: " + juce::String (outputFile.getSize()) + " bytes");
        DBG ("Duration: " + juce::String (bufferToSave.getNumSamples() / currentSampleRate, 2) + " seconds");
        DBG("=== saveRecordingToFile SUCCESS ===");
    }
    catch (const std::exception& e)
    {
        DBG ("Exception while saving file: " + juce::String (e.what()));
        statusLabel->setText ("ERROR: Exception during save!", juce::dontSendNotification);
    }
}

void RecorderComponent::validateAndProcessRecording()
{
    try
    {
        DBG("=== validateAndProcessRecording START ===");
        DBG("Buffer samples: " + juce::String(recordingBuffer.getNumSamples()));
        DBG("Sample rate: " + juce::String((int)currentSampleRate));
        
        // Create a copy of the recording buffer to avoid touching the original
        juce::AudioBuffer<float> processedBuffer;
        processedBuffer.makeCopyOf(recordingBuffer);
        
        DBG("Created buffer copy");
        
        // Trim to actual recorded length
        // Find where signal drops below noise floor (-80 dBFS)
        float threshold = std::pow (10.0f, -80.0f / 20.0f);
        int endSample = processedBuffer.getNumSamples();

        const auto* data = processedBuffer.getReadPointer (0);
        for (int i = processedBuffer.getNumSamples() - 1; i > 0; --i)
        {
            if (std::abs (data[i]) > threshold)
            {
                endSample = i;
                break;
            }
        }

        DBG("Found end sample: " + juce::String(endSample));
        
        // Trim buffer (add 100ms tail)
        int newSize = endSample + static_cast<int>(0.1 * currentSampleRate);
        if (newSize > processedBuffer.getNumSamples())
            newSize = processedBuffer.getNumSamples();
            
        processedBuffer.setSize (1, newSize, true);
        DBG("Trimmed to " + juce::String(processedBuffer.getNumSamples()) + " samples");

        // Normalize to -3 dBFS
        float maxLevel = 0.0f;
        for (int i = 0; i < processedBuffer.getNumSamples(); ++i)
            maxLevel = juce::jmax (maxLevel, std::abs (processedBuffer.getSample(0, i)));

        DBG("Max level found: " + juce::String(maxLevel, 6));

        if (maxLevel > 0.0f)
        {
            float targetDb = std::pow (10.0f, -3.0f / 20.0f);
            float gain = targetDb / maxLevel;
            DBG("Applying gain: " + juce::String(gain, 4));

            auto* writeData = processedBuffer.getWritePointer (0);
            for (int i = 0; i < processedBuffer.getNumSamples(); ++i)
                writeData[i] *= gain;
        }

        // Save to file using the processed buffer
        DBG("Calling saveRecordingToFile...");
        saveRecordingToFile (equipmentSelector->getText(), processedBuffer);

        // Move to complete step
        currentStep = InstructionPanel::Step::Complete;
        instructionPanel->setCurrentStep (currentStep);
        nextButton->setButtonText ("NEW RECORDING");
        backButton->setEnabled (false);
        recordButton->setEnabled (false);
        stopButton->setEnabled (false);

        statusLabel->setText ("✓ IR successfully saved to library!", juce::dontSendNotification);
        DBG("=== validateAndProcessRecording SUCCESS ===");
    }
    catch (const std::exception& e)
    {
        DBG("EXCEPTION in validateAndProcessRecording: " + juce::String(e.what()));
        statusLabel->setText ("ERROR: Processing failed - " + juce::String(e.what()), juce::dontSendNotification);
    }
    catch (...)
    {
        DBG("UNKNOWN EXCEPTION in validateAndProcessRecording");
        statusLabel->setText ("ERROR: Unknown error during processing", juce::dontSendNotification);
    }
}

