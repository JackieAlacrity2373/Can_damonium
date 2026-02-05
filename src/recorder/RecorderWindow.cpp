#include "RecorderWindow.h"
#include "RecorderComponent.h"

class RecorderMenuBarModel : public juce::MenuBarModel
{
public:
    RecorderMenuBarModel(RecorderComponent* comp) : component(comp) {}
    
    juce::StringArray getMenuBarNames() override
    {
        return { "File", "Audio" };
    }
    
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String&) override
    {
        juce::PopupMenu menu;
        
        if (topLevelMenuIndex == 0) // File
        {
            menu.addItem(1, "Quit");
        }
        else if (topLevelMenuIndex == 1) // Audio
        {
            menu.addItem(2, "Audio Settings...");
        }
        
        return menu;
    }
    
    void menuItemSelected(int menuItemID, int) override
    {
        if (menuItemID == 1) // Quit
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }
        else if (menuItemID == 2) // Audio Settings
        {
            if (component)
                component->showAudioSettings();
        }
    }
    
private:
    RecorderComponent* component;
};

RecorderWindow::RecorderWindow()
    : DocumentWindow ("Can Damonium IR Recorder",
                      juce::Desktop::getInstance().getDefaultLookAndFeel()
                          .findColour (juce::ResizableWindow::backgroundColourId),
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    
    auto* comp = new RecorderComponent();
    recorderComponent = comp;
    setContentOwned (comp, true);

#if JUCE_MAC
    menuBarModel.reset(new RecorderMenuBarModel(recorderComponent));
    juce::MenuBarModel::setMacMainMenu(menuBarModel.get());
#else
    setMenuBar(new RecorderMenuBarModel(recorderComponent));
#endif

    centreWithSize (900, 800);
    setVisible (true);
}

RecorderWindow::~RecorderWindow()
{
#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu(nullptr);
#else
    setMenuBar(nullptr);
#endif
}

void RecorderWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
