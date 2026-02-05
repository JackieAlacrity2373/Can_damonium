#include "StandaloneAccess.h"

#if JucePlugin_Build_Standalone
 #include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

namespace StandaloneAccess
{
    void showAudioSettingsDialog()
    {
    #if JucePlugin_Build_Standalone
        if (auto* holder = juce::StandalonePluginHolder::getInstance())
            holder->showAudioSettingsDialog();
    #endif
    }

    juce::String getAudioDeviceSummary()
    {
    #if JucePlugin_Build_Standalone
        if (auto* holder = juce::StandalonePluginHolder::getInstance())
        {
            if (auto* device = holder->deviceManager.getCurrentAudioDevice())
            {
                return device->getName()
                    + " | SR: " + juce::String (device->getCurrentSampleRate(), 0)
                    + " | Block: " + juce::String (device->getCurrentBufferSizeSamples())
                    + " | In: " + juce::String (device->getActiveInputChannels().countNumberOfSetBits())
                    + " | Out: " + juce::String (device->getActiveOutputChannels().countNumberOfSetBits());
            }
            return "No current audio device";
        }
        return "No standalone holder";
    #else
        return "Not standalone";
    #endif
    }
}
