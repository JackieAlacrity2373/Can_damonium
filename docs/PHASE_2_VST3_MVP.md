# Phase 2: MVP VST3 Plugin with Single IR

**Goal**: Build a working convolver plugin with Regular Pringles Can IR embedded

**Timeline**: 2-3 hours from IR to working plugin in DAW

---

## 📋 Project Overview

### What We're Building
A VST3 plugin that:
- Loads Regular Pringles Can IR from embedded resource
- Processes audio through convolution
- Outputs convolved signal
- Loads in any VST3-compatible DAW
- Zero configuration needed

### MVP Scope (Keep It Simple)
✓ Convolution working
✓ IR embedded
✓ No crashes
✗ No UI controls (add later)
✗ No presets (add later)
✗ No other profiles (add after MVP)

### Why This Order?
1. Get core convolution working first
2. Test end-to-end (record → plugin → DAW)
3. THEN add polish and extra profiles

---

## 🏗️ Architecture

### Plugin Structure
```
src/plugin/
├── PluginProcessor.h/cpp      (Audio processing + convolution)
├── PluginEditor.h/cpp          (UI - basic for MVP)
├── PluginComponent.h/cpp       (JUCE wrapper)
├── CMakeLists.txt              (Build config)
└── Resources/
    └── regular_pringles.wav    (Your recording)
```

### Audio Flow
```
Input Audio
    ↓
[Convolution Engine]
    ↓
Apply Regular Pringles Can IR
    ↓
Output Audio (Convolved)
```

### Signal Processing
- **Input**: 1-2 channels, any sample rate
- **Processing**: Convolution with IR
- **Output**: Same as input
- **Latency**: IR length (~5 seconds) = 240k samples at 48kHz

---

## 🚀 Development Steps

### Step 1: Prepare IR Resource

**Create Resources Directory**
```powershell
New-Item -ItemType Directory -Path h:\Can_damonium\src\plugin\Resources -Force
```

**Copy Your Recording**
```powershell
$ir_file = Get-ChildItem $env:USERPROFILE\Documents\Can_damonium\IRs\ | `
  Sort-Object LastWriteTime -Descending | Select-Object -First 1

Copy-Item $ir_file.FullName h:\Can_damonium\src\plugin\Resources\regular_pringles.wav -Force

Write-Host "✓ Copied: $($ir_file.Name)"
Write-Host "✓ Size: $($ir_file.Length / 1MB)MB"
```

### Step 2: Create Convolver Class

File: `src/plugin/ConvolverProcessor.h`

```cpp
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

class ConvolverProcessor
{
public:
    ConvolverProcessor();
    
    void prepare (double sampleRate, int blockSize);
    void process (juce::AudioBuffer<float>& buffer);
    void loadIR (const float* irData, int irLength);
    void reset();
    
    int getLatency() const { return (int)irBuffer.getNumSamples(); }
    
private:
    juce::AudioBuffer<float> irBuffer;
    juce::dsp::Convolution convolver;
    juce::dsp::ProcessorDuplicator<juce::dsp::Convolution,
                                    juce::dsp::ConvolutionMessageQueue> stereoConvolver;
};
```

### Step 3: Load IR from Resource

File: `src/plugin/PluginProcessor.cpp`

```cpp
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize convolver
    convolver.prepare (sampleRate, samplesPerBlock);
    
    // Load embedded IR
    auto& binary = BinaryData::regular_pringles_wav;
    juce::MemoryInputStream stream (binary, BinaryData::regular_pringles_wavSize, false);
    
    juce::WavAudioFormat wavFormat;
    auto reader = std::unique_ptr<juce::AudioFormatReader> (
        wavFormat.createReaderFor (&stream, false)
    );
    
    if (reader)
    {
        juce::AudioBuffer<float> irBuffer (reader->numChannels, 
                                          reader->lengthInSamples);
        reader->read (&irBuffer, 0, reader->lengthInSamples, 0, true, true);
        
        // Load first channel (mono IR)
        convolver.loadIR (irBuffer.getReadPointer(0), 
                         irBuffer.getNumSamples());
    }
}
```

### Step 4: Generate Binary Data

```powershell
# Create Jucer project script to embed resource
# Or manually add to CMakeLists.txt:

# In src/plugin/CMakeLists.txt:
juce_add_binary_data(PluginBinaryData
    SOURCES
    Resources/regular_pringles.wav
)

target_link_libraries(CanDamoniumPlugin
    PluginBinaryData
)
```

### Step 5: Process Audio

File: `src/plugin/PluginProcessor.cpp`

```cpp
void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, 
                                   juce::MidiBuffer&)
{
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Output = Input convolved with IR
    convolver.process (buffer);
}
```

### Step 6: Build and Test

**Build**
```powershell
cd h:\Can_damonium\build
cmake --build . --config Release
```

**Expected Output**
```
[100%] Built target CanDamoniumPlugin
CanDamoniumPlugin.vst3 created successfully
```

**Plugin Location**
```
h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3
```

---

## 🧪 Testing Checklist

### Pre-Testing
- [ ] IR file copied to Resources/
- [ ] Build completes without errors
- [ ] `.vst3` file exists
- [ ] DAW is VST3-compatible

### DAW Testing

**Install Plugin**
```powershell
$plugin_file = "h:\Can_damonium\build\src\plugin\Release\CanDamoniumPlugin.vst3"
$vst3_folder = "$env:PROGRAMFILES\Common Files\VST3"

Copy-Item $plugin_file $vst3_folder -Force

Write-Host "✓ Plugin installed"
Write-Host "Rescan plugins in your DAW"
```

**Load Plugin**
1. Open DAW
2. Rescan plugins
3. Look for "Can Damonium Plugin"
4. Insert on audio track

**Test Audio**
1. Feed audio into track
2. Listen to output
3. Should sound like processed through Pringles can amp
4. Try different input levels
5. Try different source material

**Success Indicators**
- ✅ No crash on load
- ✅ No crash on processing
- ✅ Audio comes out
- ✅ You hear the IR effect
- ✅ Output changes with input

### Verification

**Latency Check**
```
Expected latency: ~5-6 seconds (IR length)
This is normal for convolution
```

**Audio Quality Check**
- No clipping
- No distortion
- No artifacts
- Clean convolution

---

## 🎵 Test Audio Ideas

### What to Test With
- **Drum hit**: Single transient → hear IR ring
- **Sweep tone**: Sine sweep through IR
- **Music**: Full song through Pringles can emulation
- **Speech**: Vocals convolved with IR
- **White noise**: Full spectral response

### Expected Results
- Clean, non-destructive convolution
- Pringles can "resonance" in output
- Natural room character
- No digital artifacts

---

## 📝 Code Files to Create/Modify

### Create New Files
1. `src/plugin/ConvolverProcessor.h` (60 lines)
2. `src/plugin/ConvolverProcessor.cpp` (120 lines)

### Modify Existing
1. `src/plugin/PluginProcessor.h` - Add convolver member
2. `src/plugin/PluginProcessor.cpp` - Load IR, process audio
3. `src/plugin/CMakeLists.txt` - Embed resource

### Resources
1. `src/plugin/Resources/regular_pringles.wav` (your recording)

---

## 🚨 Troubleshooting

### Build Fails

**Error: "regular_pringles.wav not found"**
```powershell
# Verify file exists
Test-Path h:\Can_damonium\src\plugin\Resources\regular_pringles.wav
```

**Error: "Convolution header not found"**
```cpp
// Make sure JUCE DSP module is linked
// In CMakeLists.txt: juce_audio_processors, juce_dsp
```

### Plugin Doesn't Load

**Plugin not visible in DAW**
- [ ] File is in correct VST3 folder
- [ ] Rescan plugins
- [ ] Try restarting DAW
- [ ] Check DAW is 64-bit

**DAW crashes on load**
- [ ] Rebuild with `--clean-first`
- [ ] Check Visual Studio Runtime installed
- [ ] Verify IR file is valid

### No Audio Output

**Audio doesn't flow through**
- [ ] Check convolver initialization
- [ ] Verify IR loaded successfully
- [ ] Check buffer sizes match
- [ ] Try bypass first (output = input)

**Audio but no effect**
- [ ] IR may not be loaded
- [ ] Check binary data embedding
- [ ] Verify convolution is being called
- [ ] Check latency setting

---

## 📊 Performance Notes

### Expected Performance
- **CPU Usage**: 5-15% per instance (depends on IR length)
- **Latency**: 5-6 seconds (IR length, unavoidable)
- **Memory**: 50-100 MB per instance (IR buffer)
- **Compatibility**: Any VST3 DAW (Reaper, Studio One, Ableton 12+, Cubase 12+)

### Optimization (Not for MVP)
- Shorter IR = less latency
- Multithreading support
- Optimize convolution algorithm
- Add bypass for low-latency path

---

## ✅ MVP Success Criteria

- [x] IR loads without errors
- [x] Plugin instantiates in DAW
- [x] Audio passes through cleanly
- [x] Convolution is audible
- [x] No crashes during playback
- [x] Latency is acceptable
- [ ] (Not needed for MVP) UI controls
- [ ] (Not needed for MVP) Other profiles

---

## 🎯 Next Steps After MVP

### Phase 2b: Polish MVP
1. Add input/output level control
2. Add wet/dry mix
3. Show IR waveform in editor
4. Add bypass button
5. Add latency reporting

### Phase 3: Multi-Profile
1. Record Small Pringles Can
2. Record Grande Pringles Can
3. Add profile selector UI
4. Load selected IR dynamically
5. Test all three profiles

---

## 📖 Reference Documentation

- IR Standard: [IR_STANDARD.md](docs/IR_STANDARD.md)
- Recording Guide: [BUILD_AND_RECORD_GUIDE.md](docs/BUILD_AND_RECORD_GUIDE.md)
- After Recording: [AFTER_RECORDING_NEXT_STEPS.md](AFTER_RECORDING_NEXT_STEPS.md)
- JUCE Convolution: https://docs.juce.com/master/classdsp_1_1Convolution.html

---

*Phase 2 MVP Guide*
*Build a working convolver plugin with your Regular Pringles Can IR*
*Timeline: 2-3 hours*
