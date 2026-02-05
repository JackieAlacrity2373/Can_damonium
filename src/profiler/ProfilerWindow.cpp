#include "ProfilerWindow.h"
#include "ProfilerComponent.h"

ProfilerWindow::ProfilerWindow()
    : DocumentWindow ("Can Damonium Profiler",
                      juce::Desktop::getInstance().getDefaultLookAndFeel()
                          .findColour (juce::ResizableWindow::backgroundColourId),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setContentOwned (new ProfilerComponent(), true);

    centreWithSize (600, 500);
    setVisible (true);
}

ProfilerWindow::~ProfilerWindow()
{
}

void ProfilerWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
