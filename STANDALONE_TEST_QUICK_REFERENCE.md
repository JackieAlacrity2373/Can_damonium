# Quick Reference - Can Damonium Standalone Testing

## Launch Commands

### Standalone Audio App
```powershell
cd h:\Can_damonium
.\build\src\plugin\CanDamoniumPlugin_artefacts\Release\Standalone\Can\ Damonium.exe
```

### With Debug Output Capture
```powershell
cd h:\Can_damonium
.\build\src\plugin\CanDamoniumPlugin_artefacts\Release\Standalone\Can\ Damonium.exe 2>debug.log
cat debug.log  # View output
```

## What to Look For

### ✅ Working (You'll See)
```
=== PluginProcessor CONSTRUCTOR ===
=== ConvolutionEngine CONSTRUCTOR ===
=== PluginEditor CONSTRUCTOR ===
=== PluginProcessor::prepareToPlay() ===
  Sample Rate: 48000 Hz
=== ConvolutionEngine::loadImpulseResponse START ===
  File: [path]\RegularPringlesCan48k.wav
  SUCCESS: IR loaded and convolver configured
=== ConvolutionEngine::processBlock() ===
  Processing 2048 frames with convolver
```

### ❌ Problems (You'll See)
```
ERROR: IR file not found!
ERROR: ConvolutionEngine not prepared for audio processing!
[crash with no output]
[audio silent even though processBlock called]
```

## Test Procedure

1. **Launch App** → See UI window with 3 sections
2. **Select Audio Device** → Choose default input/output
3. **Click Reload Button** → Trigger IR loading (watch logs)
4. **Play Audio** → Feed signal from mic or system audio
5. **Listen** → Should hear darker/echoed convolution effect

## File Locations

| File | Location |
|------|----------|
| Standalone Exe | `build/src/plugin/.../Release/Standalone/Can Damonium.exe` |
| VST3 Plugin | `build/src/plugin/.../Release/VST3/Can Damonium.vst3` |
| IR File | `src/plugin/Resources/RegularPringlesCan48k.wav` |
| Profiler | `build/src/profiler/.../Release/Can Damonium Profiler.exe` |
| Recorder | `build/src/recorder/.../Release/Can Damonium IR Recorder.exe` |

## Rebuild After Code Changes

```powershell
cd h:\Can_damonium\build
cmake .. -G "Visual Studio 17 2022" -A x64
MSBuild Can_damonium.sln /p:Configuration=Release /p:Platform=x64
```

## Key Debug Points

| Location | What to Check |
|----------|---------------|
| `PluginProcessor::prepareToPlay()` | Sample rate/block size initialization |
| `ConvolutionEngine::loadImpulseResponse()` | IR file loading success |
| `ConvolutionEngine::processBlock()` | Continuous audio processing |
| `PluginEditor::buttonClicked()` | Reload button response |

## Audio Chain

```
Microphone/System Audio
    ↓ (via JUCE AudioAppComponent)
PluginProcessor::processBlock()
    ↓
ConvolutionEngine::processBlock()
    ↓ (JUCE Convolution Processor)
RegularPringlesCan48k.wav IR applied
    ↓
Speakers/Output
```

## Expected Behavior

- **Before**: Clean input audio
- **After**: Darker, slightly echoed version (convolved with can impulse response)

---

**Duration**: ~30 seconds to launch, select device, and hear effect
**Status**: Ready to test - all dependencies built and linked
