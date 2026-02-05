#include <JuceHeader.h>
#include "ProfilerWindow.h"

class CanDamoniumProfilerApplication : public juce::JUCEApplication
{
public:
    CanDamoniumProfilerApplication() {}

    const juce::String getApplicationName() override       { return "Can Damonium Profiler"; }
    const juce::String getApplicationVersion() override   { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override            { return true; }

    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new ProfilerWindow());
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        juce::ignoreUnused (commandLine);
    }

private:
    std::unique_ptr<ProfilerWindow> mainWindow;
};

START_JUCE_APPLICATION (CanDamoniumProfilerApplication)
