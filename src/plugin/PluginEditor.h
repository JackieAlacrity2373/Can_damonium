#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class AudioHostServices; // Forward declaration

class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::Button::Listener,
                     public juce::ComboBox::Listener,
                     public juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor& processor);
    ~PluginEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    void buttonClicked(juce::Button* button) override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    
    // Refresh IR list in dropdown (call after prepareToPlay)
    void refreshIRList();
    
    // Can flavor/size management
    void updateCanFlavor(int flavorIndex);
    void updateCanSize(int sizeIndex);
    juce::Colour getCurrentFlavorColor() const;
    
    // Layout management
    void createUIComponents();
    void rebuildLayout();

private:
    PluginProcessor& processor;
    AudioHostServices* hostServices = nullptr; // May be null (VST3 mode)

    // UI Components
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::Label> irStatusLabel;
    std::unique_ptr<juce::Label> sampleRateLabel;
    std::unique_ptr<juce::Label> audioStatusLabel;
    std::unique_ptr<juce::ComboBox> irSelector;
    std::unique_ptr<juce::ComboBox> canFlavorSelector;
    std::unique_ptr<juce::ComboBox> canSizeSelector;
    std::unique_ptr<juce::TextButton> irLoadButton;
    std::unique_ptr<juce::TextButton> reloadIRButton;
    std::unique_ptr<juce::ToggleButton> bypassButton;
    std::unique_ptr<juce::ToggleButton> testToneButton;
    std::unique_ptr<juce::TextButton> audioSettingsButton;
    std::unique_ptr<juce::FileChooser> irFileChooser;

    float inputMeter = 0.0f;
    float convolutionMeter = 0.0f;
    float outputMeter = 0.0f;
    
    // Can selection state
    int currentCanFlavor = 0;
    int currentCanSize = 1; // 0=Small (360x280), 1=Regular (700x550), 2=Large (870x685)
    
    // Can sizing data
    struct CanSize {
        int width;
        int height;
        const char* name;
    };
    static constexpr CanSize canSizes[3] = {
        {360, 550, "Small"},      // Fixed height, variable width
        {700, 550, "Regular"},
        {870, 550, "Large"}
    };
    
    // Pringles flavor data
    struct FlavorInfo {
        const char* name;
        uint32_t rgbColor; // 0xRRGGBB format
    };
    static constexpr FlavorInfo flavors[10] = {
        {"Original", 0xCC0000},                 // Bright Red
        {"Sour Cream & Onion", 0x228B22},       // Forest Green
        {"Cheddar Cheese", 0xFFB612},           // Golden Orange-Yellow
        {"Jalapeño", 0x3CB371},                 // Medium Sea Green
        {"Salt & Vinegar", 0x0066CC},           // Bright Blue
        {"BBQ", 0x8B4513},                      // Saddle Brown
        {"Screamin' Dill Pickle", 0x20B2AA},    // Light Sea Green/Teal
        {"Grilled Shrimp", 0xFF7F50},           // Coral/Salmon Pink
        {"Seaweed", 0x1B4D2E},                  // Dark Forest Green
        {"Soft-Shell Crab", 0x663399},          // Slate Blue/Purple
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};