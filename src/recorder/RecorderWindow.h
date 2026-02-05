#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class RecorderComponent;

class RecorderWindow : public juce::DocumentWindow
{
public:
    RecorderWindow();
    ~RecorderWindow() override;

    void closeButtonPressed() override;

private:
    RecorderComponent* recorderComponent = nullptr;
    
#if JUCE_MAC
    std::unique_ptr<juce::MenuBarModel> menuBarModel;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecorderWindow)
};
