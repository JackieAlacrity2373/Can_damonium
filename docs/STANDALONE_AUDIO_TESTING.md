# Standalone Audio Testing - Can Damonium

## Status: BUILD SUCCESSFUL ✅

### Build Summary (February 3, 2025)

**All Targets Built Successfully:**
- ✅ **Standalone Executable**: `build/src/plugin/CanDamoniumPlugin_artefacts/Release/Standalone/Can Damonium.exe`
- ✅ **VST3 Plugin**: `build/src/plugin/CanDamoniumPlugin_artefacts/Release/VST3/Can Damonium.vst3`
- ✅ **Profiler Executable**: `build/src/profiler/CanDamoniumProfiler_artefacts/Release/Can Damonium Profiler.exe`
- ✅ **Recorder Executable**: `build/src/recorder/CanDamoniumRecorder_artefacts/Release/Can Damonium IR Recorder.exe`

**Compilation Fixes Applied:**
1. Added `juce::Button::Listener` inheritance to `PluginEditor` class
2. Implemented `buttonClicked()` method for reload button callback
3. Enabled both VST3 and Standalone targets in CMakeLists.txt

### Standalone Audio Test Plan

**Executable Location:**
```
H:\Can_damonium\build\src\plugin\CanDamoniumPlugin_artefacts\Release\Standalone\Can Damonium.exe
```

**Test Procedure:**
1. Launch standalone executable
2. Configure audio input/output device in JUCE AudioAppComponent UI
3. Feed test audio signal through
4. Observe audio routing and convolution effect
5. Monitor console output for debug messages (DBG output)
6. Check IR loading status in UI

**Expected Debug Output Sequence:**
```
=== PluginProcessor CONSTRUCTOR ===
=== ConvolutionEngine CONSTRUCTOR ===
=== PluginEditor CONSTRUCTOR ===
=== PluginProcessor::prepareToPlay() ===
  Sample Rate: [RATE] Hz
  Block Size: [SIZE] samples
  Preparing ConvolutionEngine...
=== ConvolutionEngine::prepareToPlay() ===
  Sample rate: [RATE], Block size: [SIZE]
=== PluginProcessor::loadPresetProfile() ===
  Searching for IR file...
  Trying: [PATH1]
  Found: [FINAL_PATH]
=== ConvolutionEngine::loadImpulseResponse START ===
  File: [PATH]
  SUCCESS: IR loaded and convolver configured
=== PluginProcessor::processBlock() ===
  Processing [FRAMES] frames
=== ConvolutionEngine::processBlock() ===
  Processing [FRAMES] frames with convolver
```

### Known Issues Being Debugged

**Previous Issue (DAW Testing):**
- Plugin loads successfully in DAW but audio not passing through
- No convolution effect heard despite successful compilation

**Debugging Approach:**
Using standalone executable to isolate audio routing without DAW complexity:
- Direct control of audio devices via JUCE AudioAppComponent
- Real-time DBG() output to console
- Can test audio input/output independently
- Enables identification of exact failure point in audio pipeline

### Files Modified in This Phase

1. **src/plugin/PluginEditor.h**
   - Added `juce::Button::Listener` inheritance
   - Added `buttonClicked()` method declaration

2. **src/plugin/PluginEditor.cpp**
   - Implemented `buttonClicked()` handler for reload button
   - Logs button clicks to debug console

3. **src/plugin/CMakeLists.txt**
   - Added `Standalone` to FORMATS list
   - Added juce_audio_devices, juce_audio_utils to dependencies

### Next Steps

1. **Audio Device Configuration:**
   - Check JUCE default audio device selection
   - Verify sample rate matching (48kHz expected)
   - Check buffer size configuration

2. **Convolver Initialization:**
   - Verify IR file is loading (check log output)
   - Confirm convolver.prepare() succeeds
   - Check ProcessSpec configuration

3. **Signal Flow:**
   - Verify input audio reaches processBlock()
   - Check convolution is applied
   - Verify output audio routes correctly

4. **Performance:**
   - Monitor CPU usage during convolution
   - Check for buffer overruns or audio dropouts
   - Verify no clicks/pops in output

### Testing Environment

- **OS**: Windows 10/11
- **Build System**: CMake 3.24+ → Visual Studio 2022
- **Compiler**: MSVC 14.40.33807
- **JUCE Version**: Custom fork with convolution support
- **IR File**: RegularPringlesCan48k.wav (48kHz, 24-bit, mono)
- **Build Configuration**: Release x64

### Debug UI Components

**Standalone UI Features:**
- **Status Label**: "Can Damonium IR Convolver"
- **IR Status Label**: Shows current IR loading state
- **Reload Button**: Triggers IR reload for testing
- **Audio Debug Text**: "Standalone Mode - Audio Testing"

### Audio Architecture

```
Input Audio Stream
    ↓
AudioAppComponent (JUCE Standalone)
    ↓
PluginProcessor::processBlock()
    ↓
ConvolutionEngine::processBlock()
    ↓
JUCE Convolution Processor
    ↓
Output Audio Stream
```

**Key Components:**
- `juce::dsp::Convolution` - Core convolution processor
- `juce::dsp::ProcessSpec` - Audio configuration (sample rate, block size, channels)
- `juce::dsp::ProcessContextReplacing` - In-place audio processing
- `AudioAppComponent` - Standalone audio I/O management

### Console Output Location

When running standalone executable from command line:
- Debug messages appear in VS Code terminal where exe was launched
- Can redirect to file: `Can\ Damonium.exe > output.log 2>&1`
- JUCE DBG() macros visible in Debug builds when debugger attached

---

**Status**: Ready for audio testing. Launch standalone, configure audio device, feed test signal, monitor console output.
