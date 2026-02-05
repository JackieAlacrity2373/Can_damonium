#pragma once

#include <JuceHeader.h>

/**
 * Interface for standalone host services.
 * Allows the plugin editor to interact with the host (standalone app)
 * without directly accessing global singletons or audio device managers.
 * 
 * The plugin should check if these services are available before using them.
 * In VST3/plugin mode, these services may not be available.
 */
class AudioHostServices
{
public:
    virtual ~AudioHostServices() = default;
    
    /**
     * Get current audio device status as a string.
     * Returns device name, sample rate, buffer size, channel counts.
     */
    virtual juce::String getAudioDeviceStatus() const = 0;
    
    /**
     * Request that the host show its audio settings dialog.
     * This is a request - the host may choose when/how to show it.
     */
    virtual void showAudioSettings() = 0;
    
    /**
     * Check if audio device initialization had any errors.
     */
    virtual juce::String getLastInitError() const = 0;
    
    /**
     * Get list of available input device names.
     */
    virtual juce::StringArray getInputDeviceNames() const = 0;
    
    /**
     * Get list of available output device names.
     */
    virtual juce::StringArray getOutputDeviceNames() const = 0;
    
    /**
     * Get the currently selected input device name.
     */
    virtual juce::String getCurrentInputDevice() const = 0;
    
    /**
     * Get the currently selected output device name.
     */
    virtual juce::String getCurrentOutputDevice() const = 0;
    
    /**
     * Set the active input device by name.
     */
    virtual void setInputDevice (const juce::String& deviceName) = 0;
    
    /**
     * Set the active output device by name.
     */
    virtual void setOutputDevice (const juce::String& deviceName) = 0;
};
