#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * Displays step-by-step instructions and advice for recording impulse responses
 */
class InstructionPanel : public juce::Component
{
public:
    enum class Step
    {
        Setup,
        Configuration,
        Recording,
        Processing,
        Complete
    };

    InstructionPanel();
    ~InstructionPanel() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setCurrentStep (Step step);

private:
    Step currentStep = Step::Setup;
    
    juce::String getSetupInstructions() const;
    juce::String getConfigurationInstructions() const;
    juce::String getRecordingInstructions() const;
    juce::String getProcessingInstructions() const;
    juce::String getCompleteMessage() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InstructionPanel)
};
