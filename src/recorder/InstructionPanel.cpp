#include "InstructionPanel.h"

InstructionPanel::InstructionPanel()
{
}

InstructionPanel::~InstructionPanel()
{
}

void InstructionPanel::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced (10);
    
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (14.0f);

    juce::String instructions = "Instructions";
    
    switch (currentStep)
    {
        case Step::Setup:
            instructions = getSetupInstructions();
            break;
        case Step::Configuration:
            instructions = getConfigurationInstructions();
            break;
        case Step::Recording:
            instructions = getRecordingInstructions();
            break;
        case Step::Processing:
            instructions = getProcessingInstructions();
            break;
        case Step::Complete:
            instructions = getCompleteMessage();
            break;
    }

    g.drawFittedText (instructions, bounds, juce::Justification::topLeft, 10);
}

void InstructionPanel::resized()
{
    // Layout instruction elements
}

void InstructionPanel::setCurrentStep (Step step)
{
    currentStep = step;
    repaint();
}

juce::String InstructionPanel::getSetupInstructions() const
{
    return R"(STEP 1: PHYSICAL SETUP

1. Position your amplifier or audio equipment in a quiet room
   
2. Place a microphone 6-12 inches away from the speaker, perpendicular to the cone
   
3. Avoid reflective surfaces nearby - use soft materials (blankets, foam) if needed
   
4. Connect microphone to your audio interface input (line-level or preamp)
   
5. Verify audio interface shows input signal

TIPS:
- Closer microphone placement = shorter IR duration
- Use unidirectional mic if possible to reduce room reflections
- Test signal will be 14 seconds long (4s silence + 10s chirp)
)";
}

juce::String InstructionPanel::getConfigurationInstructions() const
{
    return R"(STEP 2: AUDIO INTERFACE CONFIGURATION

Verify your audio interface settings:

INPUT (Microphone):
✓ Sample Rate: 48,000 Hz (48 kHz)
✓ Bit Depth: 24-bit
✓ Input Level: -12dB to -6dB (aim for strong signal without clipping)
✓ Channel: Mono

OUTPUT (Test Signal):
✓ Sample Rate: 48,000 Hz (48 kHz)
✓ Bit Depth: 24-bit
✓ Output Level: -3dBFS (app-controlled)
✓ Channel: Mono or Stereo (mono preferred)

EQUIPMENT SETTINGS:
- Amplifier volume: Set to normal operating level
- Amplifier tone controls: Use desired settings
- Microphone preamp (if separate): Set to appropriate level
- Ensure NO digital clipping (red lights on interface)

Click NEXT when ready to proceed.
)";
}

juce::String InstructionPanel::getRecordingInstructions() const
{
    return R"(STEP 3: RECORDING SESSION

You are now ready to record!

BEFORE CLICKING RECORD:
1. Verify input levels show on the level meter
2. Close doors/windows to minimize noise
3. Mute other applications (email, notifications, etc.)
4. Make sure you won't be disturbed for 15 seconds

RECORDING PROCESS:
1. Click "RECORD" button
2. Test signal will play through your amplifier
3. Microphone will simultaneously record the response
4. Total time: ~14 seconds (automatic)
5. DO NOT touch anything during recording!

WHAT TO EXPECT:
- You'll hear the test signal (chirp/frequency sweep)
- Recording level will show in real-time
- Progress bar indicates status
- Recording stops automatically

After recording completes, the application will validate and process the IR.
)";
}

juce::String InstructionPanel::getProcessingInstructions() const
{
    return R"(STEP 4: PROCESSING

The recorded impulse response is being processed...

PROCESSING STEPS:
1. Validating file format (48 kHz, 24-bit)
2. Analyzing decay envelope
3. Trimming to appropriate length
4. Normalizing level to -3 dBFS
5. Saving to library

This typically takes 2-5 seconds.

DO NOT close the application during processing!
)";
}

juce::String InstructionPanel::getCompleteMessage() const
{
    return R"(✓ SUCCESS!

Your impulse response has been recorded and processed.

FILE SAVED:
Location: Documents\Can_damonium\IRs\

The IR is now available in the Can Damonium VST3 plugin!

NEXT STEPS:
1. Use the IR in your DAW with the Can Damonium plugin
2. Record additional IRs by clicking "NEW RECORDING"
3. Organize multiple IRs by naming them descriptively

Your IR captures the unique acoustic characteristics of your equipment.
Each recording creates a distinct sonic profile.
)";
}

