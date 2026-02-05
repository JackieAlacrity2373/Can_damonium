#include "PluginEditor.h"
#include "HostServices.h"

// External accessor for host services (defined in Main.cpp for standalone)
extern "C" AudioHostServices* getHostServicesInstance();

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    juce::Logger::writeToLog("=== PluginEditor CONSTRUCTOR START ===");
    
    // Get host services if available (standalone mode)
    try
    {
        hostServices = getHostServicesInstance();
        if (hostServices)
            juce::Logger::writeToLog("  Host services available (standalone mode)");
        else
            juce::Logger::writeToLog("  No host services (VST3 mode)");
    }
    catch (...)
    {
        juce::Logger::writeToLog("  Host services not available (VST3 mode)");
        hostServices = nullptr;
    }
    
    try
    {
        juce::Logger::writeToLog("  About to createUIComponents...");
        // Create all UI components FIRST (before setSize which triggers resized())
        createUIComponents();
        juce::Logger::writeToLog("  createUIComponents completed");
        
        // THEN set initial window size (Regular can by default)
        // This will trigger resized() which calls rebuildLayout()
        juce::Logger::writeToLog("  About to setSize...");
        setSize (canSizes[1].width, canSizes[1].height);
        juce::Logger::writeToLog("  Size set to: " + juce::String(getWidth()) + "x" + juce::String(getHeight()));
        
        juce::Logger::writeToLog("  About to startTimerHz...");
        startTimerHz (30);
        juce::Logger::writeToLog("  Meter timer started");
        juce::Logger::writeToLog("=== PluginEditor CONSTRUCTOR SUCCESS ===");
    }
    catch (const std::exception& e)
    {
        juce::Logger::writeToLog("=== PluginEditor CONSTRUCTOR EXCEPTION: " + juce::String(e.what()) + " ===");
        throw;
    }
}

void PluginEditor::createUIComponents()
{
    juce::Logger::writeToLog("=== Creating UI Components START ===");
    
    try {
    // Status display
    juce::Logger::writeToLog("  Creating statusLabel...");
    statusLabel = std::make_unique<juce::Label> ("Status", "Can Damonium IR Convolver");
    statusLabel->setFont (juce::Font (20.0f, juce::Font::bold));
    addAndMakeVisible (*statusLabel);
    juce::Logger::writeToLog("  Status label added");
    
    // Can Flavor Selector dropdown
    canFlavorSelector = std::make_unique<juce::ComboBox> ("CanFlavorSelector");
    canFlavorSelector->addListener (this);
    for (int i = 0; i < 10; ++i)
    {
        canFlavorSelector->addItem (flavors[i].name, i + 1);
    }
    canFlavorSelector->setSelectedItemIndex (0, juce::dontSendNotification);
    canFlavorSelector->setTooltip("Choose Pringles can flavor - click palette icon");
    addAndMakeVisible (*canFlavorSelector);
    juce::Logger::writeToLog("  Can flavor selector added with 10 flavors");
    
    // Can Size Selector dropdown
    canSizeSelector = std::make_unique<juce::ComboBox> ("CanSizeSelector");
    canSizeSelector->addListener (this);
    canSizeSelector->addItem ("Small (4.75\")", 1);
    canSizeSelector->addItem ("Regular (9.25\")", 2);
    canSizeSelector->addItem ("Large (11.5\")", 3);
    canSizeSelector->setSelectedItemIndex (1, juce::dontSendNotification);
    canSizeSelector->setTooltip("Choose can size - window will resize");
    addAndMakeVisible (*canSizeSelector);
    juce::Logger::writeToLog("  Can size selector added");
    
    // IR status
    irStatusLabel = std::make_unique<juce::Label> ("IRStatus", "IR Status: Default loading...");
    irStatusLabel->setFont (juce::Font (14.0f));
    addAndMakeVisible (*irStatusLabel);
    DBG("  IR status label added");

    // Sample rate status
    sampleRateLabel = std::make_unique<juce::Label> ("SampleRate", "Sample Rate: -- Hz  |  Block: --");
    sampleRateLabel->setFont (juce::Font (12.0f));
    addAndMakeVisible (*sampleRateLabel);

    // Audio device status
    audioStatusLabel = std::make_unique<juce::Label> ("AudioStatus", "Audio: --");
    audioStatusLabel->setFont (juce::Font (12.0f));
    addAndMakeVisible (*audioStatusLabel);
    DBG("  Sample rate and audio status labels added");

    // IR Selector dropdown
    irSelector = std::make_unique<juce::ComboBox> ("IRSelector");
    irSelector->addListener (this);
    
    const auto& irs = processor.getIRLibrary().getAvailableIRs();
    DBG("PluginEditor constructor: Found " + juce::String(irs.size()) + " IRs in library");
    for (int i = 0; i < irs.size(); ++i)
    {
        DBG("  Adding IR to combo: " + irs[i].name);
        irSelector->addItem (irs[i].name, i + 1);
    }
    
    if (irs.size() > 0)
    {
        irSelector->setSelectedItemIndex (0, juce::dontSendNotification);
        DBG("  Selected first IR: " + irs[0].name);
        processor.setDeferredIRLoad(irs[0].file);
        if (irStatusLabel)
            irStatusLabel->setText ("IR Status: Loading...",
                                    juce::NotificationType::dontSendNotification);
    }
    else
    {
        DBG("  WARNING: No IRs found to populate dropdown!");
    }
    
    addAndMakeVisible (*irSelector);

    // Load custom IR button (+)
    irLoadButton = std::make_unique<juce::TextButton> ("+");
    irLoadButton->setTooltip ("Load custom IR");
    irLoadButton->addListener (this);
    irLoadButton->setColour (juce::TextButton::buttonColourId, juce::Colours::darkgreen);
    addAndMakeVisible (*irLoadButton);
    DBG("  IR load button added");
    
    // Reload button for testing
    reloadIRButton = std::make_unique<juce::TextButton> ("Reload IR");
    reloadIRButton->addListener (this);
    addAndMakeVisible (*reloadIRButton);
    DBG("  Reload button added");
    
    // Bypass button
    bypassButton = std::make_unique<juce::ToggleButton> ("Bypass: OFF");
    bypassButton->addListener (this);
    bypassButton->setToggleState (false, juce::NotificationType::dontSendNotification);
    addAndMakeVisible (*bypassButton);
    DBG("  Bypass button added (initial state: OFF)");
    
    // Test tone button (for debugging)
    testToneButton = std::make_unique<juce::ToggleButton> ("Test Tone: OFF");
    testToneButton->addListener (this);
    testToneButton->setToggleState (false, juce::NotificationType::dontSendNotification);
    addAndMakeVisible (*testToneButton);
    DBG("  Test tone button added");
    
    // Audio Settings button - opens JUCE's AudioDeviceSelectorComponent
    audioSettingsButton = std::make_unique<juce::TextButton> ("Audio Settings");
    audioSettingsButton->addListener (this);
    audioSettingsButton->setTooltip ("Configure audio device, sample rate, and channel selection");
    addAndMakeVisible (*audioSettingsButton);
    DBG("  Audio Settings button added");
    
    juce::Logger::writeToLog("=== UI Components Created SUCCESS ===");
    } catch (const std::exception& e) {
        juce::Logger::writeToLog("=== ERROR in createUIComponents: " + juce::String(e.what()) + " ===");
        throw;
    }
}

void PluginEditor::rebuildLayout()
{
    int w = getWidth();
    int h = getHeight();
    bool isSmall = (w <= 400);
    
    DBG("rebuildLayout() - Size: " + juce::String(w) + "x" + juce::String(h) + " (isSmall=" + juce::String(isSmall ? "true" : "false") + ")");
    DBG("  statusLabel: " + juce::String(statusLabel ? "OK" : "NULL"));
    DBG("  canFlavorSelector: " + juce::String(canFlavorSelector ? "OK" : "NULL"));
    DBG("  canSizeSelector: " + juce::String(canSizeSelector ? "OK" : "NULL"));
    DBG("  irStatusLabel: " + juce::String(irStatusLabel ? "OK" : "NULL"));
    DBG("  irSelector: " + juce::String(irSelector ? "OK" : "NULL"));
    
    if (isSmall)
    {
        // COMPACT LAYOUT FOR SMALL CANS (360×280)
        int y = 5;
        int xMargin = 8;
        int colWidth = (w - xMargin * 2) / 2;
        
        // Smaller title
        statusLabel->setFont (juce::Font (14.0f, juce::Font::bold));
        statusLabel->setBounds (xMargin, y, w - xMargin * 2, 22);
        y += 25;
        
        // Color palette icon (small) + Size selector on one row
        canFlavorSelector->setBounds (xMargin, y, 32, 28);  // Tiny square (palette icon area)
        canSizeSelector->setBounds (xMargin + 35, y, colWidth - 35, 28);
        y += 32;
        
        // Hide verbose status labels in small mode
        irStatusLabel->setFont(juce::Font(10.0f));
        irStatusLabel->setBounds (xMargin, y, w - xMargin * 2, 16);
        y += 18;
        
        // Hide sample rate and audio status (too verbose for small)
        sampleRateLabel->setVisible(false);
        audioStatusLabel->setVisible(false);
        
        // IR selector (compact)
        irSelector->setBounds (xMargin, y, w - xMargin * 2 - 32, 26);
        irLoadButton->setBounds (w - xMargin - 28, y, 26, 26);
        y += 30;
        
        // Button row (stacked vertically, smaller)
        int buttonH = 24;
        int buttonGap = 2;
        
        reloadIRButton->setBounds (xMargin, y, w - xMargin * 2, buttonH);
        y += buttonH + buttonGap;
        
        bypassButton->setBounds (xMargin, y, (colWidth - buttonGap) / 2, buttonH);
        testToneButton->setBounds (xMargin + (colWidth - buttonGap) / 2 + buttonGap, y, (colWidth - buttonGap) / 2, buttonH);
        testToneButton->setButtonText("Tone");  // Abbreviated for space
        y += buttonH + buttonGap;
        
        audioSettingsButton->setBounds (xMargin, y, w - xMargin * 2, buttonH);
    }
    else
    {
        // REGULAR LAYOUT FOR REGULAR/LARGE CANS
        int y = 5;
        int xMargin = 15;
        int colWidth = (w - xMargin * 2 - 20) / 2;
        
        // Full title
        statusLabel->setFont (juce::Font (20.0f, juce::Font::bold));
        statusLabel->setBounds (xMargin, y, w - xMargin * 2, 35);
        y += 40;
        
        // Flavor (with palette icon indicator) + Size selector
        canFlavorSelector->setBounds (xMargin, y, colWidth, 30);
        canSizeSelector->setBounds (xMargin + colWidth + 15, y, colWidth, 30);
        y += 35;
        
        // Status labels
        irStatusLabel->setFont(juce::Font(14.0f));
        irStatusLabel->setBounds (xMargin, y, w - xMargin * 2, 25);
        y += 28;
        
        sampleRateLabel->setVisible(true);
        sampleRateLabel->setFont(juce::Font(12.0f));
        sampleRateLabel->setBounds (xMargin, y, w - xMargin * 2, 20);
        y += 23;
        
        audioStatusLabel->setVisible(true);
        audioStatusLabel->setFont(juce::Font(12.0f));
        audioStatusLabel->setBounds (xMargin, y, w - xMargin * 2, 20);
        y += 23;
        
        // IR selector row
        irSelector->setBounds (xMargin + 55, y, w - xMargin * 2 - 100, 30);
        irLoadButton->setBounds (w - xMargin - 40, y, 30, 30);
        y += 35;
        
        // Control buttons
        int buttonW = (w - xMargin * 2 - 10) / 3;
        
        reloadIRButton->setBounds (xMargin, y, buttonW, 35);
        bypassButton->setBounds (xMargin + buttonW + 5, y, buttonW, 30);
        bypassButton->setBounds (xMargin + buttonW + 5, y, 120, 30);
        testToneButton->setBounds (xMargin + buttonW + 130, y, 150, 30);
        testToneButton->setButtonText("Test Tone: OFF");  // Full text
        audioSettingsButton->setBounds (w - xMargin - 120, y, 120, 30);
    }
}


PluginEditor::~PluginEditor()
{
    DBG("=== PluginEditor DESTRUCTOR ===");
    stopTimer();
}

void PluginEditor::paint (juce::Graphics& g)
{
    static bool loggedOnce = false;
    if (!loggedOnce)
    {
        juce::Logger::writeToLog("=== PAINT METHOD CALLED ===");
        juce::Logger::writeToLog("  Window size: " + juce::String(getWidth()) + "x" + juce::String(getHeight()));
        juce::Logger::writeToLog("  Component count: " + juce::String(getNumChildComponents()));
        loggedOnce = true;
    }
    
    // Use selected flavor color as background tint
    auto flavorColor = getCurrentFlavorColor();
    auto bgColor = juce::Colours::darkgrey.interpolatedWith(flavorColor, 0.15f);
    g.fillAll (bgColor);
    
    // Draw palette icon next to flavor selector if small
    int w = getWidth();
    if (w <= 400 && canFlavorSelector)
    {
        // Draw palette icon in the flavor selector button area
        auto paletteArea = canFlavorSelector->getBounds().toFloat();
        
        // Palette brush icon outline
        g.setColour (juce::Colours::white);
        g.drawRoundedRectangle (paletteArea, 3.0f, 1.5f);
        
        // Draw small color circles (palette colors) inside
        float cx = paletteArea.getCentreX();
        float cy = paletteArea.getCentreY();
        float radius = 2.5f;
        
        // Top colors
        g.setColour (juce::Colour(0xE31937)); // Red
        g.fillEllipse (cx - 6, cy - 5, radius * 2, radius * 2);
        
        g.setColour (juce::Colour(0x2E8B57)); // Green
        g.fillEllipse (cx, cy - 5, radius * 2, radius * 2);
        
        // Bottom colors
        g.setColour (juce::Colour(0xFF8C00)); // Orange
        g.fillEllipse (cx - 6, cy + 1, radius * 2, radius * 2);
        
        g.setColour (juce::Colour(0x20B2AA)); // Teal
        g.fillEllipse (cx, cy + 1, radius * 2, radius * 2);
        
        // Palette thumb (brown)
        g.setColour (juce::Colour(0xA0522D)); // Brown thumb
        g.fillEllipse (cx + 6, cy - 2, radius * 1.5f, radius * 1.5f);
    }
    
    g.setColour (juce::Colours::white);
    g.setFont (18.0f);
    if (w > 400)  // Only show full title in regular/large mode
    {
        g.drawText ("Can Damonium", 10, 10, w - 20, 40, juce::Justification::centred);
    }

    // Meters positioned in lower portion, accounting for dynamic window size
    int meterY = getHeight() - 140;
    if (meterY < 100) meterY = 100;  // Safety fallback
    
    auto meterArea = juce::Rectangle<int> (20, meterY, getWidth() - 40, 110);
    auto meterHeight = 20;
    auto meterGap = 8;

    auto drawMeter = [&g] (juce::Rectangle<int> bounds, const juce::String& label, float value)
    {
        auto labelArea = bounds.removeFromLeft (80);
        g.setColour (juce::Colours::white);
        g.setFont (12.0f);
        g.drawText (label, labelArea, juce::Justification::centredLeft);

        auto meterBounds = bounds.reduced (4, 4);
        g.setColour (juce::Colours::black);
        g.fillRect (meterBounds);

        auto clamped = juce::jlimit (0.0f, 1.0f, value);
        auto fillWidth = juce::roundToInt (meterBounds.getWidth() * clamped);
        auto fill = meterBounds.withWidth (fillWidth);

        g.setColour (clamped > 0.9f ? juce::Colours::red : (clamped > 0.6f ? juce::Colours::orange : juce::Colours::green));
        g.fillRect (fill);
        g.setColour (juce::Colours::grey);
        g.drawRect (meterBounds);
    };

    auto meter1 = juce::Rectangle<int> (meterArea.getX(), meterArea.getY(), meterArea.getWidth(), meterHeight);
    auto meter2 = meter1.translated (0, meterHeight + meterGap);
    auto meter3 = meter2.translated (0, meterHeight + meterGap);

    drawMeter (meter1, "Input", inputMeter);
    drawMeter (meter2, "Conv", convolutionMeter);
    drawMeter (meter3, "Out", outputMeter);
}

void PluginEditor::resized()
{
    DBG("resized() called - rebuilding layout for new size");
    rebuildLayout();
    repaint();
}

void PluginEditor::timerCallback()
{
    auto toMeter = [] (float linear) -> float
    {
        if (linear <= 0.0f)
            return 0.0f;

        auto db = juce::Decibels::gainToDecibels (linear, -60.0f);
        return juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    };

    inputMeter = toMeter (processor.getInputLevel());
    convolutionMeter = toMeter (processor.getConvolutionLevel());
    outputMeter = toMeter (processor.getOutputLevel());

    if (irStatusLabel)
    {
        irStatusLabel->setText (processor.isIrLoaded() ? "IR Status: Loaded" : "IR Status: Not loaded",
                                juce::NotificationType::dontSendNotification);
    }

    if (sampleRateLabel)
    {
        auto sr = processor.getCurrentSampleRateHz();
        auto bs = processor.getCurrentBlockSize();
        auto callbacks = processor.getAudioCallbackCount();
        auto prepCount = processor.getPrepareToPlayCount();
        auto layoutIn = processor.getLastLayoutInputs();
        auto layoutOut = processor.getLastLayoutOutputs();

        auto text = (sr > 0.0)
            ? ("Sample Rate: " + juce::String (sr, 0) + " Hz  |  Block: " + juce::String (bs)
               + "  |  Callbacks: " + juce::String (static_cast<long long> (callbacks))
               + "  |  prepareToPlay: " + juce::String (prepCount)
               + "  |  Layout: " + juce::String (layoutIn) + "->" + juce::String (layoutOut))
            : (juce::String ("Sample Rate: -- Hz  |  Block: --  |  Callbacks: 0")
               + "  |  prepareToPlay: " + juce::String (prepCount)
               + "  |  Layout: " + juce::String (layoutIn) + "->" + juce::String (layoutOut));

        sampleRateLabel->setText (text, juce::NotificationType::dontSendNotification);

        if (audioStatusLabel)
        {
            if (hostServices)
                audioStatusLabel->setText ("Audio: " + hostServices->getAudioDeviceStatus(), 
                                           juce::NotificationType::dontSendNotification);
            else
                audioStatusLabel->setText ("Audio: VST3 mode", juce::NotificationType::dontSendNotification);
        }
        
        // Refresh IR list on first audio (sample rate > 0)
        static bool refreshedOnce = false;
        if (sr > 0.0 && !refreshedOnce)
        {
            refreshedOnce = true;
            DBG("First audio detected, refreshing IR list...");
            refreshIRList();
        }
    }

    repaint();
}

void PluginEditor::buttonClicked(juce::Button* button)
{
    if (button == reloadIRButton.get())
    {
        DBG("=== RELOAD IR BUTTON CLICKED ===");
        irStatusLabel->setText("IR: Choose file...", juce::NotificationType::dontSendNotification);

        irFileChooser = std::make_unique<juce::FileChooser> (
            "Select an IR WAV file",
            juce::File::getCurrentWorkingDirectory(),
            "*.wav");

        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        irFileChooser->launchAsync (flags, [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file.existsAsFile())
            {
                DBG("  Loading IR: " + file.getFullPathName());
                processor.loadImpulseResponse (file);
                processor.getIRLibrary().addCustomIR (file);
                
                // Add to dropdown
                auto& lib = processor.getIRLibrary();
                const auto& irs = lib.getAvailableIRs();
                irSelector->clear (juce::dontSendNotification);
                for (int i = 0; i < irs.size(); ++i)
                {
                    irSelector->addItem (irs[i].name, i + 1);
                }
                irSelector->setSelectedItemIndex (irs.size() - 1);
                
                irStatusLabel->setText("IR: Loaded " + file.getFileName(), juce::NotificationType::dontSendNotification);
            }
            else
            {
                irStatusLabel->setText("IR: Load cancelled", juce::NotificationType::dontSendNotification);
            }
        });
    }
    else if (button == irLoadButton.get())
    {
        DBG("=== IR LOAD BUTTON (+) CLICKED ===");
        irStatusLabel->setText("IR: Choose file...", juce::NotificationType::dontSendNotification);

        irFileChooser = std::make_unique<juce::FileChooser> (
            "Select an IR WAV file",
            juce::File::getCurrentWorkingDirectory(),
            "*.wav");

        auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
        irFileChooser->launchAsync (flags, [this] (const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file.existsAsFile())
            {
                DBG("  Loading custom IR: " + file.getFullPathName());
                processor.loadImpulseResponse (file);
                processor.getIRLibrary().addCustomIR (file);
                
                // Add to dropdown
                auto& lib = processor.getIRLibrary();
                const auto& irs = lib.getAvailableIRs();
                irSelector->clear (juce::dontSendNotification);
                for (int i = 0; i < irs.size(); ++i)
                {
                    irSelector->addItem (irs[i].name, i + 1);
                }
                irSelector->setSelectedItemIndex (irs.size() - 1);
                
                irStatusLabel->setText("IR: Loaded " + file.getFileName(), juce::NotificationType::dontSendNotification);
            }
            else
            {
                irStatusLabel->setText("IR: Load cancelled", juce::NotificationType::dontSendNotification);
            }
        });
    }
    else if (button == audioSettingsButton.get())
    {
        DBG("=== AUDIO SETTINGS BUTTON CLICKED ===");
        if (hostServices != nullptr)
        {
            DBG("  Opening audio device selector dialog...");
            hostServices->showAudioSettings();
        }
        else
        {
            DBG("  ERROR: hostServices is nullptr!");
        }
    }
    else if (button == bypassButton.get())
    {
        bool isBypass = bypassButton->getToggleState();
        DBG("=== BYPASS BUTTON TOGGLED: " << (isBypass ? "ON" : "OFF") << " ===");
        processor.setConvolutionBypass (isBypass);
        bypassButton->setButtonText (isBypass ? "Bypass: ON" : "Bypass: OFF");
    }
    else if (button == testToneButton.get())
    {
        bool isEnabled = testToneButton->getToggleState();
        DBG("=== TEST TONE TOGGLED: " << (isEnabled ? "ON" : "OFF") << " ===");
        processor.setTestToneEnabled(isEnabled);
        testToneButton->setButtonText(isEnabled ? "Test Tone: ON" : "Test Tone: OFF");
    }
}

void PluginEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == canFlavorSelector.get())
    {
        int selectedIndex = canFlavorSelector->getSelectedItemIndex();
        DBG("=== CAN FLAVOR CHANGED: " + juce::String(flavors[selectedIndex].name) + " ===");
        updateCanFlavor(selectedIndex);
    }
    else if (comboBoxThatHasChanged == canSizeSelector.get())
    {
        int selectedIndex = canSizeSelector->getSelectedItemIndex();
        DBG("=== CAN SIZE CHANGED: " + juce::String(canSizes[selectedIndex].name) + " ===");
        updateCanSize(selectedIndex);
    }
    else if (comboBoxThatHasChanged == irSelector.get())
    {
        int selectedIndex = irSelector->getSelectedItemIndex();
        DBG("=== IR SELECTOR CHANGED: index " << selectedIndex << " ===");
        
        if (selectedIndex >= 0)
        {
            auto& lib = processor.getIRLibrary();
            const auto& irs = lib.getAvailableIRs();
            
            if (selectedIndex < irs.size())
            {
                const auto& selectedIR = irs[selectedIndex];
                DBG("  Selected IR: " + selectedIR.name);
                DBG("  Loading from: " + selectedIR.file.getFullPathName());
                DBG("  File exists: " << (selectedIR.file.existsAsFile() ? "true" : "false"));
                
                processor.loadImpulseResponse (selectedIR.file);
                
                // Check actual state after loading
                bool irLoaded = processor.isIrLoaded();
                DBG("  After load - isIrLoaded(): " << (irLoaded ? "true" : "false"));
                
                irStatusLabel->setText(irLoaded ? "IR Status: Loaded" : "IR Status: Load Failed",
                                        juce::NotificationType::dontSendNotification);
                DBG("  Status label updated");
            }
            else
            {
                DBG("  ERROR: selectedIndex " << selectedIndex << " >= irs.size() " << irs.size());
            }
        }
    }
}

void PluginEditor::refreshIRList()
{
    if (!irSelector)
        return;
        
    DBG("PluginEditor::refreshIRList() called");
    auto& lib = processor.getIRLibrary();
    const auto& irs = lib.getAvailableIRs();
    
    DBG("  Found " + juce::String(irs.size()) + " IRs");
    
    irSelector->clear (juce::dontSendNotification);
    for (int i = 0; i < irs.size(); ++i)
    {
        DBG("  Adding: " + irs[i].name);
        irSelector->addItem (irs[i].name, i + 1);
    }
    
    if (irs.size() > 0)
    {
        irSelector->setSelectedItemIndex (0, juce::dontSendNotification);
        DBG("  Selected first IR");
        processor.loadImpulseResponse (irs[0].file);
        if (irStatusLabel)
            irStatusLabel->setText (processor.isIrLoaded() ? "IR Status: Loaded" : "IR Status: Not loaded",
                                    juce::NotificationType::dontSendNotification);
    }
}
void PluginEditor::updateCanFlavor(int flavorIndex)
{
    if (flavorIndex < 0 || flavorIndex >= 10)
        return;
    
    currentCanFlavor = flavorIndex;
    DBG("Can flavor changed to: " + juce::String(flavors[flavorIndex].name));
    repaint();
}

void PluginEditor::updateCanSize(int sizeIndex)
{
    if (sizeIndex < 0 || sizeIndex >= 3)
        return;
    
    currentCanSize = sizeIndex;
    auto newSize = canSizes[sizeIndex];
    
    DBG("Can size changed to: " + juce::String(newSize.name) + 
        " (" + juce::String(newSize.width) + "x" + juce::String(newSize.height) + ")");
    
    // Resize the window
    setSize(newSize.width, newSize.height);
}

juce::Colour PluginEditor::getCurrentFlavorColor() const
{
    if (currentCanFlavor < 0 || currentCanFlavor >= 10)
        return juce::Colours::darkgrey;
    
    auto rgb = flavors[currentCanFlavor].rgbColor;
    auto r = (uint8_t)((rgb >> 16) & 0xFF);
    auto g = (uint8_t)((rgb >> 8) & 0xFF);
    auto b = (uint8_t)(rgb & 0xFF);
    
    return juce::Colour(r, g, b);
}