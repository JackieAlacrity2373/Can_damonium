#pragma once

#include <JuceHeader.h>

/**
 * Manages audio recording and file I/O for impulse response capture
 */
class IRRecorder : public juce::AudioAppComponent
{
public:
    IRRecorder();
    ~IRRecorder() override;

    // Audio recording control
    void startRecording (const juce::String& profileName);
    void stopRecording();
    void playTestSignal();
    void stopPlayback();

    // Configuration
    void setOutputDevice (const juce::String& deviceName);
    void setInputDevice (const juce::String& deviceName);

    // Query
    bool isRecording() const { return recording; }
    bool isPlaying() const { return playing; }
    float getRecordingLevel() const;
    juce::File getLastRecordingFile() const { return lastRecordingFile; }

    // Audio processing callbacks
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    // Audio routing
    bool recording = false;
    bool playing = false;

    // Recording buffer
    juce::AudioBuffer<float> recordingBuffer;
    int recordingPosition = 0;
    const int RECORDING_SAMPLE_RATE = 48000;
    const int RECORDING_DURATION_SECONDS = 14; // 4s silence + 10s chirp

    // Test signal generation
    class TestSignalAudioSource : public juce::PositionableAudioSource
    {
    public:
        TestSignalAudioSource();
        void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
        void releaseResources() override {}
        int64_t getNextReadPosition() const override { return 0; }
        void setNextReadPosition (int64_t newPosition) override {}
        bool isLooping() const override { return false; }
        void setLooping (bool) override {}
        int64_t getTotalLength() const override { return 0; }

    private:
        // Test signal implementation
    };

    juce::File lastRecordingFile;
    juce::File profilesDirectory;

    void saveRecordingToFile (const juce::String& profileName);
    void initializeProfilesDirectory();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IRRecorder)
};
