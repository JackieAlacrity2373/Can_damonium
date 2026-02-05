#include <juce_gui_extra/juce_gui_extra.h>
#include "RecorderWindow.h"

class CanDamoniumRecorderApplication : public juce::JUCEApplication
{
public:
    CanDamoniumRecorderApplication() {}

    const juce::String getApplicationName() override       { return "Can Damonium IR Recorder"; }
    const juce::String getApplicationVersion() override   { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override            { return true; }

    void initialise (const juce::String& commandLine) override
    {
        mainWindow.reset (new RecorderWindow());
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
    std::unique_ptr<RecorderWindow> mainWindow;
};

START_JUCE_APPLICATION (CanDamoniumRecorderApplication)
