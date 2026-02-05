# Audio Debugging Protocol - Can Damonium Standalone

## Phase: Audio Pass-Through Testing

### Current Status
- ✅ VST3 Plugin: Compiles, loads in DAW, **but no audio passes through**
- ✅ Standalone App: Built successfully, UI ready for testing
- ⏳ Audio Issue: Unknown break point in signal chain

### Audio Signal Flow Diagram

```
┌─────────────────────────────────────────────────────────────┐
│ INPUT AUDIO STREAM (from device or DAW)                    │
└─────────────────────────────────────────────────────────────┘
                           ↓
┌─────────────────────────────────────────────────────────────┐
│ PluginProcessor::processBlock()                             │
│  - Receives AudioBuffer<float>                              │
│  - Calls convolutionEngine->processBlock()                  │
│  - Returns modified buffer                                  │
└─────────────────────────────────────────────────────────────┘
                           ↓
┌─────────────────────────────────────────────────────────────┐
│ ConvolutionEngine::processBlock()                           │
│  - Wraps buffer in juce::dsp::AudioBlock                    │
│  - Creates ProcessContextReplacing                          │
│  - Calls convolver.process(context)                         │
│  - Should modify buffer in-place                            │
└─────────────────────────────────────────────────────────────┘
                           ↓
┌─────────────────────────────────────────────────────────────┐
│ juce::dsp::Convolution::process()                           │
│  - Applies impulse response to audio                        │
│  - Should output convolved audio                            │
└─────────────────────────────────────────────────────────────┘
                           ↓
┌─────────────────────────────────────────────────────────────┐
│ OUTPUT AUDIO STREAM (to DAW/device speakers)                │
│  - Should hear convolution effect (darker sound)            │
└─────────────────────────────────────────────────────────────┘
```

### Critical Questions to Answer

**Question 1: Is IR file loading?**
- **Location**: ConvolutionEngine::loadImpulseResponse() → Check log for "SUCCESS: IR loaded"
- **Expected**: `RegularPringlesCan48k.wav` should be found and loaded
- **If False**: IR path resolution broken, convolver has empty IR

**Question 2: Is prepareToPlay() called?**
- **Location**: ConvolutionEngine::prepareToPlay() → Check log for sample rate/block size
- **Expected**: Should see "Sample rate: 44100/48000, Block size: 256-2048"
- **If False**: Convolver never initialized, will crash on processBlock

**Question 3: Is processBlock() called?**
- **Location**: ConvolutionEngine::processBlock() → Check log for "Processing [N] frames"
- **Expected**: Should see continuous messages during audio playback
- **If False**: No audio reaching processor, check DAW/plugin routing

**Question 4: Are buffer channels correct?**
- **Location**: ConvolutionEngine::processBlock() → Check channel count in logs
- **Expected**: 2 channels (stereo) or 1 channel (mono)
- **If False**: Channel mismatch may cause silent output

### Debugging Strategy for Standalone

#### Step 1: Launch with Debug Output
```powershell
cd h:\Can_damonium
.\build\src\plugin\CanDamoniumPlugin_artefacts\Release\Standalone\Can\ Damonium.exe 2>console_output.txt
```

#### Step 2: Check Initial Logs
**Expected in console immediately after launch:**
```
=== PluginProcessor CONSTRUCTOR ===
=== ConvolutionEngine CONSTRUCTOR ===
=== PluginEditor CONSTRUCTOR ===
```

#### Step 3: Configure Audio Device
- In UI, look for audio device selector (JUCE standard)
- Select default input/output device
- Click UI to enable audio processing

#### Step 4: Check Prepare Logs
**Expected after audio device starts:**
```
=== PluginProcessor::prepareToPlay() ===
  Sample Rate: [48000] Hz
  Block Size: [2048] samples
  Preparing ConvolutionEngine...
=== ConvolutionEngine::prepareToPlay() ===
  Sample rate: [48000], Block size: [2048]
```

#### Step 5: Test Audio Input
- Play audio through system input
- Or use test tone generator
- Check if processBlock logs appear:
```
=== ConvolutionEngine::processBlock() ===
  Processing [2048] frames with convolver
```

#### Step 6: Verify Output
- Listen to output
- Should hear convolution effect (audio sounds darker/echoed)
- Or record output and analyze waveform

### Common Issues and Solutions

| Issue | Symptom | Solution |
|-------|---------|----------|
| **IR Not Loading** | IR loading logs missing | Check file path in loadPresetProfile() |
| **Convolver Not Init** | prepareToPlay logs missing | Verify sample rate is valid (not 0) |
| **No Audio Flow** | processBlock never called | Check audio device routing |
| **Silent Output** | processBlock called but silent | Check convolver.process() working |
| **Crash on Start** | App crashes at launch | Check JuceHeader.h includes or dependencies |

### Files to Monitor During Testing

**Code Files with Logging:**
- [src/plugin/PluginProcessor.cpp](../src/plugin/PluginProcessor.cpp#L1) - Main processor, logs prepareToPlay & processBlock
- [src/plugin/ConvolutionEngine.cpp](../src/plugin/ConvolutionEngine.cpp#L1) - Convolver, logs IR loading & processing
- [src/plugin/PluginEditor.cpp](../src/plugin/PluginEditor.cpp#L1) - UI, logs button clicks

**Configuration Files:**
- [src/plugin/CMakeLists.txt](../src/plugin/CMakeLists.txt#L1) - Build configuration

### Testing Checklist

- [ ] Standalone executable launches without crashing
- [ ] Sees console "CONSTRUCTOR" messages
- [ ] Audio device selector appears in UI
- [ ] Selects audio device and starts audio stream
- [ ] Sees "prepareToPlay()" messages with sample rate
- [ ] Sees "loadImpulseResponse()" messages with IR filename
- [ ] Plays test audio and sees "processBlock()" messages
- [ ] Hears convolution effect on output (or records it)
- [ ] No audio stuttering or dropouts
- [ ] No CPU spikes during processing

### Next Actions After Testing

**If Audio Works in Standalone:**
1. Return to DAW plugin and verify same behavior
2. Check DAW buffer size configuration matches
3. Test in actual music production scenario

**If Audio Fails in Standalone:**
1. Locate exact break point from log messages
2. Add more detailed logging to that section
3. Check JUCE configuration for audio routing
4. Verify convolver.prepare() succeeds
5. Test with simpler pass-through (no convolution)

### Hypothesis: Why No Audio in DAW

**Most Likely:**
- IR file path not found when running in DAW (relative path issue)
- Convolver not initialized due to sample rate mismatch
- Buffer channel count mismatch (mono IR vs stereo processing)

**Less Likely:**
- Processor disconnected from audio graph
- VST3 parameter/port configuration wrong
- JUCE audio module configuration issue

**Standalone Will Prove:**
- Whether IR loading works
- Whether convolver processes audio
- Whether JUCE audio I/O is configured correctly

---

**Next Step**: Launch standalone, monitor console output, identify where audio breaks, report findings with specific log lines.
