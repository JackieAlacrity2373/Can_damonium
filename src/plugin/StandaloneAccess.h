#pragma once

#include <JuceHeader.h>

// Standalone-only helpers to access JUCE StandalonePluginHolder safely.
namespace StandaloneAccess
{
    // Opens the JUCE audio settings dialog (standalone builds only).
    void showAudioSettingsDialog();

    // Returns a human-readable summary of current audio device state.
    juce::String getAudioDeviceSummary();
}
