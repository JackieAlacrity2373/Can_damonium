#pragma once

#include <JuceHeader.h>

class ProfilerWindow : public juce::DocumentWindow
{
public:
    ProfilerWindow();
    ~ProfilerWindow() override;

    void closeButtonPressed() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfilerWindow)
};
