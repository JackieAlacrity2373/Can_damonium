#include "IRRecorder.h"

IRRecorder::IRRecorder()
{
    initializeProfilesDirectory();
    
    // Set up audio device manager
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.sampleRate = RECORDING_SAMPLE_RATE;
    setup.bufferSize = 512;
    deviceManager.setAudioDeviceSetup (setup, true);
    
    juce::String result = deviceManager.initialiseWithDefaultDevices (1, 1);
    if (result.isNotEmpty())
        DBG ("Audio device error: " << result);
    
    // Allocate recording buffer (14 seconds at 48 kHz)
    recordingBuffer.setSize (1, RECORDING_SAMPLE_RATE * RECORDING_DURATION_SECONDS);
}

IRRecorder::~IRRecorder()
{
    shutdownAudio();
}

void IRRecorder::startRecording (const juce::String& profileName)
{
    recording = true;
    recordingPosition = 0;
    recordingBuffer.clear();
    
    // Start audio playback and recording
    setAudioChannels (1, 1);
}

void IRRecorder::stopRecording()
{
    recording = false;
    // Save recording to file
    saveRecordingToFile ("Recorded_IR");
}

void IRRecorder::playTestSignal()
{
    playing = true;
}

void IRRecorder::stopPlayback()
{
    playing = false;
}

void IRRecorder::setOutputDevice (const juce::String& deviceName)
{
    // TODO: Set output device
}

void IRRecorder::setInputDevice (const juce::String& deviceName)
{
    // TODO: Set input device
}

float IRRecorder::getRecordingLevel() const
{
    if (recordingPosition == 0) return 0.0f;
    
    auto* data = recordingBuffer.getReadPointer (0);
    float maxLevel = 0.0f;
    
    for (int i = 0; i < recordingPosition; ++i)
        maxLevel = std::max (maxLevel, std::abs (data[i]));
    
    return maxLevel;
}

void IRRecorder::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (recording && recordingPosition < recordingBuffer.getNumSamples())
    {
        auto* input = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);
        auto* recordBuffer = recordingBuffer.getWritePointer (0);
        
        int samplesToRecord = bufferToFill.numSamples;
        if (recordingPosition + samplesToRecord > recordingBuffer.getNumSamples())
            samplesToRecord = recordingBuffer.getNumSamples() - recordingPosition;
        
        std::copy (input, input + samplesToRecord, recordBuffer + recordingPosition);
        recordingPosition += samplesToRecord;
    }
}

void IRRecorder::releaseResources()
{
    // No resources to release yet
}

void IRRecorder::saveRecordingToFile (const juce::String& profileName)
{
    // TODO: Write WAV file with recording
}

void IRRecorder::initializeProfilesDirectory()
{
    profilesDirectory = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Can_damonium")
        .getChildFile ("profiles");
    
    if (!profilesDirectory.exists())
        profilesDirectory.createDirectory();
}

// Inner class implementation
IRRecorder::TestSignalAudioSource::TestSignalAudioSource()
{
}

void IRRecorder::TestSignalAudioSource::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // TODO: Generate test signal
    bufferToFill.clearActiveBufferRegion();
}

void IRRecorder::TestSignalAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    juce::ignoreUnused (samplesPerBlockExpected, sampleRate);
}
