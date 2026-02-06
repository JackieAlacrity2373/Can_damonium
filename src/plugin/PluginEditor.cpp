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

    // IR resample toggle
    resampleIrButton = std::make_unique<juce::ToggleButton> ("IR Resample: ON");
    resampleIrButton->addListener (this);
    resampleIrButton->setToggleState (processor.isIrResampleEnabled(), juce::NotificationType::dontSendNotification);
    addAndMakeVisible (*resampleIrButton);
    resampleIrButton->setButtonText(processor.isIrResampleEnabled() ? "IR Resample: ON" : "IR Resample: OFF");
    DBG("  IR resample toggle added");
    
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
    // Use current can size to determine layout instead of width
    // This ensures consistent layout regardless of manual resizing
    bool isSmall = (currentCanSize == 0);
    
    DBG("rebuildLayout() - Size: " + juce::String(w) + "x" + juce::String(h) + " (isSmall=" + juce::String(isSmall ? "true" : "false") + ")");
    DBG("  statusLabel: " + juce::String(statusLabel ? "OK" : "NULL"));
    DBG("  canFlavorSelector: " + juce::String(canFlavorSelector ? "OK" : "NULL"));
    DBG("  canSizeSelector: " + juce::String(canSizeSelector ? "OK" : "NULL"));
    DBG("  irStatusLabel: " + juce::String(irStatusLabel ? "OK" : "NULL"));
    DBG("  irSelector: " + juce::String(irSelector ? "OK" : "NULL"));
    
    // Fixed layout with variable width - all sizes now have fixed height of 550px
    int y = 5;
    int xMargin = isSmall ? 8 : 15;
    int titleFontSize = isSmall ? 14 : 20;
    int titleHeight = isSmall ? 22 : 35;
    
    // TITLE
    statusLabel->setFont(juce::Font(static_cast<float>(titleFontSize), juce::Font::bold));
    statusLabel->setBounds(xMargin, y, w - xMargin * 2, titleHeight);
    y += titleHeight + 5;
    
    if (isSmall)
    {
        // SMALL LAYOUT (360px width)
        // Color palette icon (small) + Size selector on one row
        canFlavorSelector->setBounds(xMargin, y, 32, 28);
        canSizeSelector->setBounds(xMargin + 35, y, w - xMargin * 2 - 35, 28);
        y += 32;
        
        // Hide verbose status labels in small mode
        irStatusLabel->setFont(juce::Font(10.0f));
        irStatusLabel->setBounds(xMargin, y, w - xMargin * 2, 16);
        irStatusLabel->setVisible(true);
        y += 18;
        
        sampleRateLabel->setVisible(false);
        audioStatusLabel->setVisible(false);
        
        // IR selector (compact) - ensure always visible
        int irSelectorWidth = juce::jmax(100, w - xMargin * 2 - 32);
        irSelector->setBounds(xMargin, y, irSelectorWidth, 26);
        irSelector->setVisible(true);
        irLoadButton->setBounds(xMargin + irSelectorWidth + 2, y, 26, 26);
        irLoadButton->setVisible(true);
        y += 30;
        
        // Control buttons
        int buttonH = 24;
        int buttonGap = 2;
        
        reloadIRButton->setBounds(xMargin, y, w - xMargin * 2, buttonH);
        y += buttonH + buttonGap;
        
        audioSettingsButton->setBounds(xMargin, y, w - xMargin * 2, buttonH);
        y += buttonH + buttonGap + 20;
        
        // Bypass and Tone buttons
        int halfWidth = (w - xMargin * 2 - buttonGap) / 2;
        bypassButton->setBounds(xMargin, y, halfWidth, buttonH);
        testToneButton->setBounds(xMargin + halfWidth + buttonGap, y, halfWidth, buttonH);
        testToneButton->setButtonText("Tone");
        y += buttonH + buttonGap;

        // IR resample toggle
        resampleIrButton->setBounds(xMargin, y, w - xMargin * 2, buttonH);
    }
    else
    {
        // REGULAR/LARGE LAYOUT (700px+ width)
        int colWidth = (w - xMargin * 2 - 20) / 2;
        
        // Flavor + Size selector on same row
        canFlavorSelector->setBounds(xMargin, y, colWidth, 30);
        canSizeSelector->setBounds(xMargin + colWidth + 15, y, colWidth, 30);
        y += 35;
        
        // Status labels - all visible in regular/large
        irStatusLabel->setFont(juce::Font(14.0f));
        irStatusLabel->setBounds(xMargin, y, w - xMargin * 2, 25);
        y += 28;
        
        sampleRateLabel->setVisible(true);
        sampleRateLabel->setFont(juce::Font(12.0f));
        sampleRateLabel->setBounds(xMargin, y, w - xMargin * 2, 20);
        y += 23;
        
        audioStatusLabel->setVisible(true);
        audioStatusLabel->setFont(juce::Font(12.0f));
        audioStatusLabel->setBounds(xMargin, y, w - xMargin * 2, 20);
        y += 23;
        
        // IR selector row - ensure width never goes negative
        int irSelectorWidth = juce::jmax(150, w - xMargin * 2 - 100);
        int irSelectorX = xMargin + (w - xMargin * 2 - irSelectorWidth - 35) / 2; // Center if narrower than expected
        irSelector->setBounds(irSelectorX, y, irSelectorWidth, 30);
        irSelector->setVisible(true);
        irLoadButton->setBounds(irSelectorX + irSelectorWidth + 5, y, 30, 30);
        irLoadButton->setVisible(true);
        y += 35;
        
        // Control buttons - adapt to width
        int buttonGap = 5;
        int availableWidth = w - xMargin * 2;
        int buttonH = 30;
        
        // For wider screens, distribute buttons better
        if (w >= 700)
        {
            // Wider layout: Reload | Bypass | Test Tone | IR Resample | Audio Settings
            int buttonW = (availableWidth - buttonGap * 4) / 5;
            reloadIRButton->setBounds(xMargin, y, buttonW, buttonH);
            bypassButton->setBounds(xMargin + buttonW + buttonGap, y, buttonW, buttonH);
            testToneButton->setBounds(xMargin + (buttonW + buttonGap) * 2, y, buttonW, buttonH);
            resampleIrButton->setBounds(xMargin + (buttonW + buttonGap) * 3, y, buttonW, buttonH);
            testToneButton->setButtonText("Test Tone: OFF");
            audioSettingsButton->setBounds(xMargin + (buttonW + buttonGap) * 4, y, buttonW, buttonH);
        }
        else
        {
            // Narrower layout - stack buttons vertically
            reloadIRButton->setBounds(xMargin, y, availableWidth, buttonH);
            y += buttonH + buttonGap;
            bypassButton->setBounds(xMargin, y, availableWidth / 2 - buttonGap/2, buttonH);
            testToneButton->setBounds(xMargin + availableWidth / 2 + buttonGap/2, y, availableWidth / 2 - buttonGap/2, buttonH);
            y += buttonH + buttonGap;
            resampleIrButton->setBounds(xMargin, y, availableWidth / 2 - buttonGap/2, buttonH);
            audioSettingsButton->setBounds(xMargin + availableWidth / 2 + buttonGap/2, y, availableWidth / 2 - buttonGap/2, buttonH);
        }
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

    // Draw only the convolution (2nd) meter
    auto meterRect = juce::Rectangle<int> (meterArea.getX(), meterArea.getY(), meterArea.getWidth(), meterHeight);
    drawMeter (meterRect, "Conv Output", convolutionMeter);
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
    
    // Only update IR status if it changed
    static bool lastIRLoaded = false;
    if (irStatusLabel && processor.isIrLoaded() != lastIRLoaded)
    {
        lastIRLoaded = processor.isIrLoaded();
        irStatusLabel->setText (lastIRLoaded ? "IR Status: Loaded" : "IR Status: Not loaded",
                                juce::NotificationType::dontSendNotification);
    }

    // Only update sample rate label every few frames (less frequently)
    static int updateCounter = 0;
    if (++updateCounter >= 5 && sampleRateLabel)  // Update every ~167ms instead of ~33ms
    {
        updateCounter = 0;
        
        auto sr = processor.getCurrentSampleRateHz();
        auto bs = processor.getCurrentBlockSize();
        auto callbacks = processor.getAudioCallbackCount();
        auto prepCount = processor.getPrepareToPlayCount();
        auto layoutIn = processor.getLastLayoutInputs();
        auto layoutOut = processor.getLastLayoutOutputs();

        // Only update if sample rate is active
        if (sr > 0.0)
        {
            auto text = "Sample Rate: " + juce::String (sr, 0) + " Hz  |  Block: " + juce::String (bs)
                       + "  |  Callbacks: " + juce::String (static_cast<long long> (callbacks));
            sampleRateLabel->setText (text, juce::NotificationType::dontSendNotification);

            if (audioStatusLabel)
            {
                if (hostServices)
                    audioStatusLabel->setText ("Audio: " + hostServices->getAudioDeviceStatus(), 
                                               juce::NotificationType::dontSendNotification);
            }
        }
        
        // Refresh IR list on first audio (sample rate > 0)
        static bool refreshedOnce = false;
        if (sr > 0.0 && !refreshedOnce)
        {
            refreshedOnce = true;
            juce::Logger::writeToLog("First audio detected, refreshing IR list...");
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
    else if (button == resampleIrButton.get())
    {
        bool isEnabled = resampleIrButton->getToggleState();
        DBG("=== IR RESAMPLE TOGGLED: " << (isEnabled ? "ON" : "OFF") << " ===");
        processor.setIrResampleEnabled(isEnabled);
        resampleIrButton->setButtonText(isEnabled ? "IR Resample: ON" : "IR Resample: OFF");
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
    
    // All IRs are available in all UI sizes - no filtering
    // (IR selection is independent of can size UI)
    
    // Auto-resize on first selection of Small or Large (one-time feature)
    bool shouldAutoResize = false;
    
    if (sizeIndex == 0 && !smallCanResizedOnce)
    {
        smallCanResizedOnce = true;
        shouldAutoResize = true;
        DBG("First Small selection - auto-resizing");
    }
    else if (sizeIndex == 2 && !largeCanResizedOnce)
    {
        largeCanResizedOnce = true;
        shouldAutoResize = true;
        DBG("First Large selection - auto-resizing");
    }
    
    if (shouldAutoResize)
    {
        // Resize on first selection of Small or Large
        setSize(newSize.width, newSize.height);
    }
    else if (sizeIndex == 1)
    {
        // Regular size can always resize
        setSize(newSize.width, newSize.height);
    }
    else
    {
        // For subsequent Small/Large selections, just update layout without resizing
        resized();
    }
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