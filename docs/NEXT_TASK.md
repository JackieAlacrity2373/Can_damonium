# Next Task: WAV File I/O Implementation

## What's Complete
✅ Full user-facing IR recorder application with 5-step workflow
✅ Test signal generator (logarithmic chirp)
✅ Audio recording pipeline (input capture to buffer)
✅ Real-time UI with level metering and progress
✅ Processing pipeline (trimming, normalization)

## What's Needed: WAV File Writing

The application captures IRs in memory but needs to **save them to WAV files**.

### Location: RecorderComponent.cpp
Method: `saveRecordingToFile()`
Currently: TODO comment, needs implementation

### Requirements
**Input**: 
- Recording buffer (1 channel, 48000 Hz, 24-bit, variable duration)
- Equipment name (string)

**Output**:
- WAV file written to `Documents\Can_damonium\IRs\{name}_{timestamp}_48k.wav`

**WAV Format**:
- Sample rate: 48,000 Hz
- Channels: 1 (mono)
- Bit depth: 24-bit PCM
- Header: Standard RIFF WAV structure

### JUCE API Options

**Option 1: Using JUCE AudioFormatWriter** (Recommended)
```cpp
auto format = std::make_unique<juce::WavAudioFormat>();
juce::FileOutputStream outStream(outputFile);
auto writer = format->createWriterFor(&outStream, 48000, 1, 24, {}, 0);
writer->writeFromAudioSampleBuffer(recordingBuffer, 0, recordingBuffer.getNumSamples());
```

**Option 2: Manual WAV Header + Write**
- More control, more code to write

### Acceptance Criteria
1. ✓ File created in correct directory
2. ✓ Filename format: `{equipment}_{timestamp}_48k.wav`
3. ✓ File is valid 48kHz, 24-bit, mono WAV
4. ✓ Can be opened in audio editor
5. ✓ Can be re-imported into the recorder app for verification

---

## Testing Plan

Once WAV I/O is implemented:

1. **Build** the recorder application
2. **Record** one Small Pringles Can IR as test
3. **Verify** the saved WAV file:
   - Open in Audacity/DAW
   - Check: 48 kHz sample rate
   - Check: Mono channel
   - Check: Frequency content (20Hz-20kHz sweep visible)
   - Check: ~4-6 seconds duration (after trimming)
4. **Record** Regular and Grande profiles
5. **Then** proceed to VST3 plugin implementation

---

## Implementation Estimate
- **Complexity**: Low-Medium
- **Time**: 30-60 minutes
- **Dependencies**: JUCE WavAudioFormat
- **Testing**: Manual verification with audio editor

Would you like me to implement the WAV file I/O next?
